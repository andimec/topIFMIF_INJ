#!/usr/bin/env python3
"""Select literal records without rewriting their definitions. Python stdlib only.

Input: JSON list of {path, content, sha} original DB files, registry JSON,
and output directory. Output is a reviewable reference subset, not a runnable IOC.
"""
import argparse
import collections
import hashlib
import json
import re
from pathlib import Path

TOKEN = re.compile(r'#[^\r\n]*|"(?:\\.|[^"\\])*"|[{}(),]|[^\s{}(),"#]+', re.S)

def records(text):
    tokens = [m for m in TOKEN.finditer(text) if not m.group().startswith('#')]
    result = []
    i = 0
    while i < len(tokens):
        if tokens[i].group() not in ('record', 'grecord'):
            i += 1
            continue
        start = i
        assert [t.group() for t in tokens[i+1:i+2]] == ['(']
        assert tokens[i+3].group() == ','
        assert tokens[i+5].group() == ')'
        assert tokens[i+6].group() == '{'
        name = tokens[i+4].group().strip('"')
        kind = tokens[i+2].group().strip('"')
        i += 7
        depth = 1
        while depth and i < len(tokens):
            v = tokens[i].group()
            depth += (v == '{') - (v == '}')
            i += 1
        assert depth == 0, name
        raw = text[tokens[start].start():tokens[i-1].end()]
        fields = {}
        for j in range(start+7, i-4):
            if tokens[j].group() == 'field' and tokens[j+1].group() == '(':
                fields[tokens[j+2].group().strip('"')] = tokens[j+4].group().strip('"')
        result.append(dict(name=name, record_type=kind, raw=raw, fields=fields,
                           line=text.count('\n', 0, tokens[start].start())+1))
    return result

def generate(sources, registry, output):
    output = Path(output)
    names = {p['name'] for p in registry['pvs']}
    assert len(names) == len(registry['pvs'])
    by_name = collections.defaultdict(list)
    templates = []
    all_records = []
    for source in sources:
        parsed = records(source['content'])
        # Every non-comment record keyword must have been consumed.
        assert len(parsed) == sum(t.group() in ('record', 'grecord') for t in TOKEN.finditer(source['content']) if not t.group().startswith('#'))
        for record in parsed:
            record.update(path=source['path'], source_blob_sha=source['sha'])
            all_records.append(record)
            if '$' in record['name']:
                templates.append(record)
            else:
                by_name[record['name']].append(record)
    selected = {}
    unresolved = []
    coverage = []
    for name in sorted(names):
        base = name if name in by_name else name.rsplit('.', 1)[0]
        candidates = by_name.get(base, [])
        signatures = {tuple(t.group() for t in TOKEN.finditer(r['raw']) if not t.group().startswith('#')) for r in candidates}
        if len(signatures) == 1:
            r = sorted(candidates, key=lambda x: (x['path'], x['line']))[0]
            selected[(r['path'], r['line'])] = r
            coverage.append(dict(pv=name, record=base, path=r['path'], line=r['line'],
                                 status='literal_record' if name == base else 'record_field',
                                 record_type=r['record_type'], fields=r['fields'],
                                 record_sha256=hashlib.sha256(r['raw'].encode()).hexdigest()))
        else:
            patterns = []
            for r in templates:
                # A fully macro-defined name matches everything and is not evidence.
                literal = re.sub(r'\$\([^)]*\)|\$\{[^}]*\}', '', r['name'])
                if not re.search(r'[A-Za-z0-9]', literal):
                    continue
                pattern = re.escape(r['name'])
                pattern = re.sub(r'\\\$\\\([^)]*?\\\)|\\\$\\\{[^}]*?\\\}', '.+?', pattern)
                if re.fullmatch(pattern, name.rsplit('.', 1)[0] if '.' in name else name):
                    patterns.append(dict(path=r['path'], line=r['line'], template=r['name']))
            unresolved.append(dict(pv=name, reason='conflicting_definitions' if candidates else 'macro_unresolved' if patterns else 'not_found_in_db',
                                   candidates=[dict(path=r['path'], line=r['line'], record_type=r['record_type']) for r in candidates],
                                   possible_templates=patterns))
    files = collections.defaultdict(list)
    for r in selected.values():
        files[r['path']].append(r)
    for path, rs in files.items():
        target = output / path
        target.parent.mkdir(parents=True, exist_ok=True)
        content = '# Inventory V1.2 reference subset. Original record bodies preserved.\n# Not a standalone IOC; see reports/pv_selection.md.\n\n' + '\n\n'.join(r['raw'] for r in sorted(rs, key=lambda x: x['line'])) + '\n'
        target.write_bytes(content.encode())
        assert [r['raw'] for r in records(content)] == [r['raw'] for r in sorted(rs, key=lambda x: x['line'])]
    links = []
    selected_names = {r['name'] for r in selected.values()}
    for r in selected.values():
        for field, value in r['fields'].items():
            if field in ('INP', 'OUT', 'DOL', 'FLNK', 'SDIS', 'TSEL') or re.fullmatch(r'INP[A-U]', field):
                target = value.split()[0] if value.split() else ''
                if ':' in target and not target.startswith(('@', '#')) and '$' not in target:
                    base = target.split('.')[0]
                    if base not in selected_names:
                        links.append(dict(pv=r['name'], field=field, value=value, path=r['path']))
    report = dict(inventory_count=len(names), source_db_count=len(sources), source_records=len(all_records),
                  retained_db_count=len(files), retained_records=len(selected), covered_pvs=len(coverage),
                  unresolved_pvs=len(unresolved), coverage=coverage, unresolved=unresolved,
                  unresolved_templates=[{k:r[k] for k in ('name','path','line')} for r in templates],
                  links_outside_subset=links,
                  removed_db_files=sorted(s['path'] for s in sources if s['path'] not in files))
    (output/'reports').mkdir(parents=True, exist_ok=True)
    (output/'reports/pv_selection.json').write_text(json.dumps(report, indent=2)+'\n')
    return report

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('sources')
    parser.add_argument('registry')
    parser.add_argument('output')
    args = parser.parse_args()
    report = generate(json.loads(Path(args.sources).read_text()), json.loads(Path(args.registry).read_text()), args.output)
    print(json.dumps({k:v for k,v in report.items() if isinstance(v, int)}, indent=2))

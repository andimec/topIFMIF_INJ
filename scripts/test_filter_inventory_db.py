import importlib.util
import tempfile
import unittest
from pathlib import Path
spec = importlib.util.spec_from_file_location('selector', Path(__file__).with_name('filter_inventory_db.py'))
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)

class SelectionTests(unittest.TestCase):
    def test_parser_preserves_quotes_comments_crlf_and_braces(self):
        raw = 'record(ao, "A:B") {\r\n field(DESC, "brace } # text")\r\n # }\r\n field(HOPR, "20")\r\n}'
        parsed = m.records('# record(ai, "FAKE") {}\n'+raw)
        self.assertEqual(len(parsed), 1)
        self.assertEqual(parsed[0]['raw'], raw)
        self.assertEqual(parsed[0]['fields']['DESC'], 'brace } # text')
    def test_selection_excludes_conflicts_and_does_not_guess_macros(self):
        sources = [dict(path='a/db/a.db', sha='test', content='record(ao,A:B) {field(HOPR,"20")}\nrecord(ai,A:C) {}\nrecord(ai,"$(P):D") {}'), dict(path='b/db/b.db',sha='test',content='record(bi,A:C) {}')]
        registry = dict(pvs=[dict(name=n) for n in ['A:B','A:B.HOPR','A:C','A:D','Z:UNKNOWN']])
        with tempfile.TemporaryDirectory() as d:
            r=m.generate(sources,registry,d)
            self.assertEqual(r['covered_pvs'],2)
            self.assertEqual(r['retained_records'],1)
            self.assertEqual({p['pv']:p['reason'] for p in r['unresolved']},{'A:C':'conflicting_definitions','A:D':'macro_unresolved','Z:UNKNOWN':'not_found_in_db'})
            self.assertFalse((Path(d)/'b/db/b.db').exists())
    def test_malformed_record_fails_closed(self):
        with self.assertRaises(AssertionError):
            m.records('record(ai, A:B) { field(VAL,"1")')

if __name__=='__main__':
    unittest.main()

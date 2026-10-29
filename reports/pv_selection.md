# Selección de PVs del inventario V1.2

## Resultado

- Inventario: 891 entradas únicas.
- Origen: 46 archivos `.db`, 729 registros activos.
- Selección: 30 archivos `.db`, 596 registros, 598 entradas cubiertas.
- Pendientes: 293 entradas. 149 sin coincidencia literal ni plantilla candidata específica, 140 con posibles plantillas sin macros verificadas y 4 con definiciones incompatibles.

## Criterio de conservación

Se conservan las rutas originales y literalmente los bloques `record` seleccionados, incluidos comentarios internos, tipo, límites de visualización y control, alarmas, unidades, precisión, SCAN, DTYP, INP/OUT, FLNK y demás campos. No se inventan campos ausentes ni se cambian valores por defecto. Los comentarios externos y metadatos gráficos VisualDCT no se trasladan. Los `.db` sin registros seleccionados se eliminan únicamente en esta rama. Los archivos ajenos a `.db` permanecen como en la base.

Las entradas `LEBT:FC:ICalc.INPE` y `LEBT:MPS:HVsparkSuppressor:delay.HIGH` están cubiertas por sus registros padre, cuyos campos aparecen explícitamente en el original. No se crean registros con el sufijo de campo en el nombre.

Una coincidencia de plantilla es solo una pista, no una identificación verificada. No se deducen valores de macros a partir del nombre del inventario. No se encontraron archivos de sustitución ni scripts de carga de IOC en el árbol fuente que permitan verificar esas instancias. Los cuatro nombres con conflictos se excluyen de todas sus ubicaciones hasta decidir qué definición corresponde.

## Alcance de uso

Esta rama es una referencia filtrada de las definiciones originales, no una Soft IOC ejecutable ni una implementación del runtime Python. Mantiene nombres originales, drivers y lógica EPICS. No añade `OTP:`, no convierte DTYP a Soft Channel y no sustituye SCAN. La adaptación al gemelo digital requiere un paso posterior. Los Makefiles originales pueden referenciar bases eliminadas. No se ha compilado ni arrancado un IOC.

`pv_selection.json` contiene trazabilidad por PV, campos originales, hashes SHA-256 de cada bloque, posibles plantillas, archivos retirados y enlaces fuera del subconjunto. `provenance.json` fija el commit fuente y el blob del inventario. La copia del inventario está en `config/pv_registry_v1_2.json`.

## Validación

Se ha comprobado que cada bloque emitido coincide literalmente con el bloque fuente, que todos los registros emitidos corresponden al inventario o a un registro padre necesario, que no hay nombres duplicados y que cobertura más pendientes suma 891. Las pruebas del selector comprueban comentarios, cadenas con llaves, CRLF, campos, macros sin resolver, conflictos y sintaxis incompleta.

Reproducción: obtener los 46 archivos indicados en `provenance.json` desde el commit fuente y preparar una lista JSON con `path`, `sha` y `content`; ejecutar `python scripts/filter_inventory_db.py source_dbs.json config/pv_registry_v1_2.json DIRECTORIO_NUEVO`. Usar un directorio vacío para no dejar resultados antiguos. Pruebas: `python scripts/test_filter_inventory_db.py`.

## Conflictos que requieren decisión

- `LEBT:SOL01_PS:OnStat`: `injectorApp/db/injectorSOL01.db:73` (calc); `plcInjectorApp/db/plcInjector_Input.db:161` (bi)
- `LEBT:SOL02_PS:OnStat`: `injectorApp/db/injectorSOL02.db:74` (calc); `plcInjectorApp/db/plcInjector_Input.db:195` (bi)
- `SRC:SVL:D2CloseStat`: `emitApp/db/extraPV.db:15` (bi); `plcInjectorApp/db/plcInjector_Input.db:1457` (bi)
- `SRC:SVL:H2CloseStat`: `emitApp/db/extraPV.db:25` (bi); `plcInjectorApp/db/plcInjector_Input.db:1479` (bi)

## Enlaces hacia registros fuera del subconjunto

- `LEBT:FC:IRendement` / `INPB` → `LEBT:EMIT_FC:I`.
- `LEBT:IC:IRendement` / `INPB` → `LEBT:IC:I`.
- `LEBT:SOL01_PS:IntCmd` / `OUT` → `SRC:SOL01_PS:RampeCmd.VAL PP`.
- `LEBT:SOL02_PS:IntCmd` / `OUT` → `SRC:SOL02_PS:RampeCmd.VAL PP`.

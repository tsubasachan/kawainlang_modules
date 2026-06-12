# kawainlang_modules

Modulos oficiais da KawaiLang.

## Versionamento

O conjunto de modulos e versionado por `kawai.modules.toml`.
Cada modulo tambem possui um manifesto proprio `kawai.mod.toml`, com:

- `name`: nome logico do modulo, como `std/http`.
- `version`: versao SemVer do modulo.
- `entry`: arquivo `.🎀` principal.
- `ffi`: arquivos opcionais Python/C usados pelo interpretador e compilador.
- `compat.kawailang`: versao minima da KawaiLang esperada.

## Modulos atuais

- `std/config` (`config/`): configuracao e leitura simples de `.env`.
- `std/http` (`http/`): cliente HTTP GET via FFI, parsers HTTP/URL, servidor HTTP simples e helpers de rota.
- `std/json` (`json/`): manipulacao JSON com objetos aninhados, listas e conversores.
- `kawai/web` (`kawai_web/`): KawaiWeb, framework web minimalista com templates e middleware.
- `std/log` (`log/`): logging estruturado textual.
- `std/rede` (`rede/`): sockets TCP POSIX via FFI.
- `std/result` (`result/`): tipos `Resultado` e `Option` para fluxos sem excecao no caminho normal.

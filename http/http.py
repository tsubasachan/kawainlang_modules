import urllib.request

def kl_http_get(url):
    try:
        req = urllib.request.Request(
            url, 
            headers={'User-Agent': 'KawaiLang-HTTP-Client/1.0'}
        )
        with urllib.request.urlopen(req) as response:
            raw_data = response.read()
            charset = response.info().get_content_charset() or 'utf-8'
            try:
                return raw_data.decode(charset)
            except (UnicodeDecodeError, LookupError):
                # Fallback seguro que substitui caracteres inválidos sem estourar exceção
                return raw_data.decode('utf-8', errors='replace')
    except Exception as e:
        raise RuntimeError(f"Erro na requisição HTTP: {e}")

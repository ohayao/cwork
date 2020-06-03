#!/bin/zsh
# Example on MacOS shell script using curl
## JSON Data
#D=" {
#  token: 'abcdef',
#  csr: \"$(cat ./igloo/csr.pem)\",
#  bridgeId: 'ign12345'
#} "
## Send post request


CSR="-----BEGIN CERTIFICATE REQUEST-----\n
MIICijCCAXICAQAwRTELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUx\n
ITAfBgNVBAoMGEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDCCASIwDQYJKoZIhvcN\n
AQEBBQADggEPADCCAQoCggEBAKQURh7+J+RKjHvRfy5tao+IPkgfAp8Cg57nqXjH\n
DlXtc4xAbgXmAVLEnhYNCSY5e4PUeni01qk8wiR5qTJuoSGdJKuhHbpebTJOiEBh\n
d5oM2tW0Lb+1+mbZO/qOVvB+PC9QCU5bBZ92qgEXuggrtBgEj0t/URHUEudtGzzJ\n
eRoncWluhGIzeuR86oYjw/O+ycCdsrWLVcjTKo/OF7pgHk6nZCHngt6/9yIbkqbk\n
Sk7HLqrf07U1dVE6u+R54CT0QpozYCDw0PDYznO28Fz9/oINSJR9xn2yQqUFqS10\n
15AgHy/WpDUL7WgG5/UEcDfVZhXXFxoyncPQYuLyF+rSJwkCAwEAAaAAMA0GCSqG\n
SIb3DQEBCwUAA4IBAQCOUKNbXL4GRIp2EPqZTL4ErJHwOaTLwzG7mRaOgY9NA9QE\n
1zIytUlmSUrIBDS7s61S2dkZsCS9FhCRs3Tsh6+UPD3KISokKmqbQdIvbQF23oUP\n
O4ytdQocV29AnTT15K6YW2T5IThEk+NZ9yuzcGcQg/REfQyYdNdqvbLdu7JQ+M81\n
h9W1Tt/FlB0jEHXIoN16ilmlklj2qF0/mqxWBE1lForaGe6ieczlo29zYJEkrwhm\n
p7FivW7aYHDMFJQksob8FYywsWuSrdgB1dDFl+WD3MQgm+dQk4bKztL7wPwNpruN\n
OYaQDDijinb9XcMv74uycS2bI8RzWUEXu9GNvcc6\n
-----END CERTIFICATE REQUEST-----"

D=" {
  token: 'abcdef',
  csr: '$CSR',
  bridgeId: 'ign12345'
} "

#echo $D
URL="https://pq7g2e9on6.execute-api.ap-southeast-1.amazonaws.com/development/hubProvision" 
curl --url "$URL" --header "Content-Type: application/json" --data "$D" --request POST




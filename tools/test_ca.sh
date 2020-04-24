#!/bin/zsh

## Example on MacOS shell script using curl

## JSON Data
D="{
\"token\":\"1234567890\",
\"bridgeId\":\"abcdef\",
\"csr\":\"-----BEGIN CERTIFICATE REQUEST-----\nMIICijCCAXICAQAwRTELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUx\nITAfBgNVBAoMGEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDCCASIwDQYJKoZIhvcN\nAQEBBQADggEPADCCAQoCggEBAKQURh7+J+RKjHvRfy5tao+IPkgfAp8Cg57nqXjH\nDlXtc4xAbgXmAVLEnhYNCSY5e4PUeni01qk8wiR5qTJuoSGdJKuhHbpebTJOiEBh\nd5oM2tW0Lb+1+mbZO/qOVvB+PC9QCU5bBZ92qgEXuggrtBgEj0t/URHUEudtGzzJ\neRoncWluhGIzeuR86oYjw/O+ycCdsrWLVcjTKo/OF7pgHk6nZCHngt6/9yIbkqbk\nSk7HLqrf07U1dVE6u+R54CT0QpozYCDw0PDYznO28Fz9/oINSJR9xn2yQqUFqS10\n15AgHy/WpDUL7WgG5/UEcDfVZhXXFxoyncPQYuLyF+rSJwkCAwEAAaAAMA0GCSqG\nSIb3DQEBCwUAA4IBAQCOUKNbXL4GRIp2EPqZTL4ErJHwOaTLwzG7mRaOgY9NA9QE\n1zIytUlmSUrIBDS7s61S2dkZsCS9FhCRs3Tsh6+UPD3KISokKmqbQdIvbQF23oUP\nO4ytdQocV29AnTT15K6YW2T5IThEk+NZ9yuzcGcQg/REfQyYdNdqvbLdu7JQ+M81\nh9W1Tt/FlB0jEHXIoN16ilmlklj2qF0/mqxWBE1lForaGe6ieczlo29zYJEkrwhm\np7FivW7aYHDMFJQksob8FYywsWuSrdgB1dDFl+WD3MQgm+dQk4bKztL7wPwNpruN\nOYaQDDijinb9XcMv74uycS2bI8RzWUEXu9GNvcc6\n-----END CERTIFICATE REQUEST-----\n\"
}"
#echo $D

## Send post request
H="https://pq7g2e9on6.execute-api.ap-southeast-1.amazonaws.com/development/igh/hubProvision"
#H="localhost/igh/hubProvision"
curl --url "$H" --header "Content-Type: application/json" --data "$D" --request POST

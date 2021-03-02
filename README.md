# Klient-server-aplikace-pro-prenos-soubor-
klientská a server aplikace v C realizující čtení/zápis souborů z/na server

Server nedpředpokládá přístup více klientů zároveň.


ARGUMENTY KLIENTA:
./ipk-client -h host -p port [-r|-w] file

např.:
./ipk-client -h eva.fit.vutbr.cz -p 55555 -r myfile.xml


ARGUMENTY SERVERU:
./ipk-server -p port                     

např.:
./ipk-server -p 55555 

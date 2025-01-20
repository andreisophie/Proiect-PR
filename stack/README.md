# Tema 3 SCD

by ANdrei Mărunțiș

## Descriere generală

Tema necesită implementarea unui stack de servicii complex pentru monitorizarea unei rețele de dispozitive IoT, folosind docker swarm.

Imaginile de docker folosite sunt urmatoarele:

- eclipse-mosquitto: cea mai recentă versiune
- influxdb: versiunea 1.8, deoarece versiunile 2+ nu acceptă conexiuni fără autentificare
- grafana: cea mai recentă versiune
- adapter: un mic program python care face legătura dintre broker-ul MQTT și influxdb

## Mod de rulare: scriptul run.sh

Mai întâi trebuie inițializat un swarm în care să ruleze soluția, folosind comanda `docker swarm init`.

Următorul pas este crearea imaginii pentru serviciul adapter folosind comanda `docker build . -t scd3_adapter`.

Ultimul pas pentru lansarea în execuție a codului este deploy-ul: `docker stack deploy -c stack.yml scd3`. De aici, docker swarm se va ocupe de pornirea serviciilor.

## Configurare grafana

Anumite elemente ale aplicatiei Grafana trebuie configurate separat prin cativa pasi simpli (pentru a nu folosi volume transferate intre sisteme). Mai precis:

- Trebuie adaugata sursa de date influxdb, link-ul `http://influxdb:8086` si baza de date `tema3scd`.
- Trebuie importate dashboard-urile, folosind fisierele json din folder-ul `grafana-dashboards`.

## Utilizare

Pentru a adauga date generate aleator în baza de date se poate porni scriptul python `client.py`, care va adauga o intrare nouă în baza de date la fiecare 5 secunde, folosind broker-ul MQTT. Ultreior, datele pot fi vizualizate în interfața grafică din grafana.

## Sugestii

De departe cel mai problematic aspect al temei a fost faptul că se dorește rularea serviciului grafana pe portul 80, care este frecvent ocupat de alte servicii, fiind portul default pentru conexiuni HTTP. Dacă enunțul ar fi permis rularea acestui serviciu pe un alt port, tema ar fi fost mult mai puțin frustrantă.
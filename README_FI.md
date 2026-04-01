# simppeliTCU 🍃
**Kevyt ja avoin tee-se-itse-korvaaja Nissan Leaf ZE1:n alkuperäiselle etäohjaukselle (TCU).**

Kun Nissanin alkuperäiset pilvipalvelut lakkaavat toimimasta tai muuttuvat epäluotettaviksi, `simppeliTCU` palauttaa autosi hallinnan omiin käsiisi. Tämä ESP32-pohjainen laite kytkeytyy suoraan auton IT CAN -väylään (yhteisössä usein tunnettu nimellä CAR-CAN) kuten alkuperäinen TCU ja tarjoaa helpon Wi-Fi-käyttöliittymän.

Huomio! Projekti on hahmotelmavaiheessa ja koodit suurelta osin generoitu AI työkalulla.

## Ominaisuudet
* 🌡️ **Lämmityksen etäohjaus:** Ilmastoinnin käynnistys ja välitön pakkosammutus.
* 🔋 **Latauksen etäohjaus:** Latausajastimen ohitus (latauksen aloitus)
* 📊 **Auton tila:** Lukee väylältä akun tarkan varausprosentin (SOC) ja sisälämpötilan.
* 🌐 **Web-käyttöliittymä:** Toimii suoraan puhelimen selaimella.

## Laitteistovaatimukset
1. **Lilygo T-2CAN** (ESP32-mikrokontrolleri CAN-piirillä).
2. **12V -> 5V autokäyttöön tarkoitettu jännitteenalennin** (Esim. upotettava 12V USB-pistorasia).
3. Pinnit/liittimet alkuperäisen TCU-johdon päähän.

### ⚠️ TÄRKEÄ VAROITUS VIRRANSYÖTÖSTÄ
**ÄLÄ kytke auton 12V-linjaa suoraan Lilygo-levyyn!** Auton sähköjärjestelmän jännitepiikit todennäköisesti rikkovat laitteen.
* Irrota alkuperäinen TCU IT-CAN väylästä.
* Ota 12V-virta TCU:n liittimestä ja vie se autokäyttöön tarkoitetun 12V USB-pistorasian (ja sulakkeen) läpi.
* Syötä Lilygolle virta USB-kaapelilla rasiasta (puhdas 5V).
* Kytke `CAN-H` ja `CAN-L` Lilygon TWAI-pinneihin (Port B).

## Vastuuvapauslauseke (MIT-lisenssi)
Tämä ohjelmisto on lisensoitu MIT-lisenssillä.

**VASTUUVAPAUSLAUSEKE:**
Tämä koodi ja laitteistomuutos ohjaa ajoneuvon korkeajännitejärjestelmiä. Ohjelmisto tarjotaan "SELLAISENAAN" (As Is) ilman minkäänlaisia takuita. Koodin käyttö tapahtuu täysin omalla vastuullasi. Tekijät eivät ole missään vastuussa autollesi aiheutuvista vahingoista, takuun raukeamisesta tai muista henkilö- ja omaisuusvahingoista. **KÄYTÄ OMALLA VASTUULLA.**
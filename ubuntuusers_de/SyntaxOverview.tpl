= Syntax =
{{{#!vorlage Tabelle
<-2 rowclass="titel">Syntax
+++
<rowclass="kopf">Syntax
Beschreibung
+++
`'''fett'''`
Text fett formatieren
+++
`''kursiv''`
Text kursiv formatieren
+++
`\`Monotype\``
Text mit fester Buchstabenweite formatieren
+++
`__unterstrichen__`
Text unterstreichen
+++
`--(durchstreichen)--`
Text durchstreichen
+++
`^^(hochgestellt)^^`
Text hochgestellt
+++
`,,(tiefgestellt),,`
Text tiefgestellt
+++
`~+(größer)+~`
Schrift größer darstellen
+++
`~-(kleiner)-~`
Schrift kleiner darstellen
+++
`[size=X]Text[/size]`
Text in Schriftgröße X darstellen, X wird in Pixel angegeben
+++
`[color=#ABCDEF]Farbe[/color]`
Text farbig darstellen, die Farbe wird als RGB Hex-Wert angegeben.
+++
`[color=red]Farbe[/color]`
Text farbig darstellen, die Farbe wird als standardisierter Farbname angegeben
+++
`[[Bild(name.png, Größe, Ausrichtung)]]`
Stellt das Bild "name.png" dar, "Größe" und "Ausrichtung" sind optional; "name.png" muss sich im Anhang befinden.
+++
`[[Bild(URL_zum_Bild, Größe, Ausrichtung)]]`
Stellt ein externes Bild dar, ansonsten identisch mit internem Bild; im Wiki nicht verwenden, sondern Bilder als Anhang hochladen und einbinden!
+++
`[URL]`
Link auf URL
+++
`[URL Text]`
Link auf URL, anstelle der URL wird "Text" angezeigt.
+++
`[:Wikiseite:]`
Link auf "Wikiseite" im Wiki
+++
`[:Wikiseite:Text]`
Link auf "Wikiseite" im Wiki, anstelle des Seitennames wird "Text" angezeigt.
+++
`[[Anker(Name)]]`
Erzeugt den Anker "Name" an dieser Stelle, Text ist optional.
+++
`[#Anker Text]`
Auf einen Anker oder eine Überschrift innerhalb einer Wikiseite verlinken.
+++
`[:Wikiseite#Anker:Text]`
Auf einen Anker oder eine Überschrift einer anderen Wikiseite verlinken.
+++
`[Interwiki:Suchbegriff:]`
Link auf "Suchbegriff" auf der mit "Interwiki" angegebenen Seite.
+++
`[Interwiki:Suchbegriff:alternativer Text]`
Link auf "Suchbegriff" auf der mit "Interwiki" angegebenen Seite. Anstelle des Suchbegriffs wird als Linktext aber "alternativer Text" angezeigt.
+++
`[[Anhang(name_des_anhangs.txt)]]`
Auf einen Anhang der Wikiseite verweisen.
+++
`[topic:Name:Text]`
Link ins Forum auf den Thread mit dem Namen "Name", "Text" ist optional und wird anstelle des Links angezeigt.
+++
`[post:12345:Text]`
Link ins Forum auf das Posting mit der Nummer "12345", "Text" ist optional und wird anstelle des Links angezeigt.
+++
`[forum:Name:Text]`
Link ins Forum "Name", "Text" ist optional und wird anstelle des Links angezeigt.
+++
`[paste:Nummer:Titel]`
Link auf den Eintrag "Nummer" mit dem Titel "Titel" im NoPaste Service.
+++
`[ikhaya:Name_der_Meldung:Text]`
Link auf eine Ikhaya-Meldung, "Text" wird anstelle des Namens der Meldung angezeigt, ist aber optional.
+++
`[bug:Nummer:Text]`
Link auf den Bug "Nummer" auf Launchpad, "Text" ist optional und wird anstelle des Links angezeigt.
+++
`{{{`[[BR]]`Code`[[BR]]`}}}`
Eine unformatierte Codezeile, Zeilenumbrüche werden übernommen.
+++
`{{{#!code Sprache`[[BR]]`Quelltext`[[BR]]`}}}`
stellt Quelltext mit Syntaxhervorhebung für "Sprache" dar.
+++
`> Zitat`
stellt "Zitat" als zitierten Text dar.
+++
`[[Datum(YYYY-MM-DDTHH:MM:SSZ)]]`
stellt Datum und Uhrzeit dar.
+++
` * Punkt 1`[[BR]]` * Punkt 2`
unnummerierte Liste, die Anzahl der Leerzeichen vor * bestimmen die Ebene.
+++
` 1. Punkt 1`[[BR]]` 1. Punkt 2`
nummerierte Liste, die Anzahl der Leerzeichen vor 1. bestimmen die Ebene. Kann auch mit Buchstaben genutzt werden (a.).
+++
`= Überschrift =`
fügt eine Überschrift ein, die Anzahl der Gleichheitszeichen bestimmt die Überschriftenebene (max. 5)
+++
`{{{#!vorlage Tabelle`[[BR]]Zelle 1[[BR]]Zelle 2[[BR]]`+++`[[BR]]Zelle 3[[BR]]Zelle 4[[BR]]`}}}`
erzeugt eine Tabelle.
+++
` \\ ` und [[Vorlage(Tasten, return)]] oder `[[BR]]`
erzwungener Zeilenumbruch
+++
`((Fußnote))`
Baut eine Fußnote in einen Artikel oder Thread ein.
+++
`## Kommentar`
fügt einen Kommentar ein, der nicht in der Ausgabe erscheint.
+++
`[[Inhaltsverzeichnis(X)]]`
fügt ein Inhaltsverzeichnis, generiert aus den Überschriften, ein; "X" gibt die Ebene an, bis zu der die Überschriften berücksichtigt werden.
+++
`#tag: tag1, tag2`
ordnet dem Artikel die Tags "tag1" und "tag2" zu.
}}}

= Vorlagen =
{{{#!vorlage Tabelle
<-2 rowclass="titel">Syntax
+++
<rowclass="kopf">Syntax
Beschreibung
+++
`[[Vorlage(Tasten, TASTE)]]`
stellt TASTE als Bild dar, kombinieren geht mit +.
+++
`[[Vorlage(Getestet, Ubuntuversion)]]`
fügt die "Getestet" Infobox für "Ubuntuversion" ein.
+++
`[[Vorlage(Getestet, general)]]`
fügt die "Getestet: general" Infobox ein.
+++
`[[Vorlage(Baustelle, Datum, "Bearbeiter")]]`
fügt die Infobox für Baustellen (neu erstellte Wiki-Artikel, die noch nicht fertig sind) ein, "Datum" ist das geplante Fertigstellungsdatum, "Bearbeiter" der aktuelle Autor des Wiki Artikels.
+++
`[[Vorlage(Fortgeschritten)]]`
fügt die "Artikel für Fortgeschrittene" Infobox hinzu.
+++
`[[Vorlage(Pakete, "foo bar")]]`
fügt das Paketmakro hinzu.
+++
`[[Vorlage(Ausbaufähig, "Begründung")]]`
markiert einen Wiki-Artikel als Ausbaufähig, "Begründung" ist der entsprechende Informationstext
+++
`[[Vorlage(Fehlerhaft, "Begründung")]]`
markiert einen Wiki-Artikel als Fehlerhaft, "Begründung" ist der entsprechende Informationstext
+++
`[[Vorlage(Verlassen, "Begründung")]]`
markiert einen Wiki-Artikel als Verlassen, "Begründung" ist der entsprechende Informationstext
+++
`[[Vorlage(Überarbeitung, Seite, Autor)]]`
markiert einen bestehenden Wiki-Artikel als aktuell in Überarbeitung
+++
`[[Vorlage(Archiviert, "Text")]]`
markiert einen Wiki-Artikel als Archiviert, "Text" ist ein zusätzlicher Informationstext
+++
`[[Vorlage(Fremd, Paket, "Kommentar")]]`
fügt eine Warnung vor Fremdpaketen ein
+++
`[[Vorlage(Fremd, Quelle, "Kommentar")]]`
fügt eine Warnung vor Fremdquelle ein
+++
`[[Vorlage(Fremd, Software, "Kommentar")]]`
fügt eine Warnung vor Fremdsoftware ein
+++
`[[Vorlage(PPA, Launchpad-Team/PPA-Name, key PGP-Schlüsselnummer, Ubuntuversion(en) )]]`
Block für PPA-Quellen
+++
`[[Vorlage(Fremdquelle, URL, Ubuntuversion(en), Komponente(n) )]]`
Block für Fremdquellen
+++
`[[Vorlage(Fremdquelle-auth, URL zum PGP-Key)]]`
Block für Fremdquellenauthentifikation
+++
`[[Vorlage(Fremdquelle-auth, key PGP-Schlüsselnummer)]]`
Block für Fremdquellenauthentifikation
+++
`[[Vorlage(Fremdpaket, Projekthoster, Projektname, Ubuntuversion(en))]]`
Block für Fremdpakete von Projekthostern (GetDeb, Launchpad, ...)
+++
`[[Vorlage(Fremdpaket, "Anbieter", URL zu einer Downloadübersicht, Ubuntuversion(en))]]`
Block für Fremdpakete von Herstellerseiten, etc.
+++
`[[Vorlage(Fremdpaket, "Anbieter", dl, URL zu einem Download, Ubuntuversion(en))]]`
Block für Fremdpakete mit Direktlinks auf Dateien
+++
`{{{#!vorlage Wissen`[[BR]]`Wissensvermerke`[[BR]]`}}}`
fügt den Wissensblock mit "Wissensvermerken" hinzu
+++
`{{{#!vorlage Hinweis`[[BR]]`Hinweistext`[[BR]]`}}}`
fügt eine Hinweisbox ein, "Hinweistext" wird darin ausgegeben
+++
`{{{#!vorlage Warnung`[[BR]]`Warntext`[[BR]]`}}}`
fügt eine Warnbox ein, "Warntext" wird darin ausgegeben
+++
`{{{#!vorlage Befehl`[[BR]]`Befehl`[[BR]]`}}}`
formatiert die Ausgabe von "Befehl" ähnlich einer Terminalausgabe
+++
`{{{#!vorlage Paketinstallation`[[BR]]`Pakete`[[BR]]`}}}`
formatiert die Ausgabe von zu installierenden Paketen inklusive weiterer Angaben
}}}
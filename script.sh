#!/bin/bash

# Verificăm dacă a fost furnizat un singur argument
if [ $# -ne 1 ]
 then
    echo "Nu avem doar un argument"
fi

# Caracterul primit ca argument
caracter=$1

# Contor pentru propozițiile corecte
count=0

# Citire continuă de la intrarea standard
while IFS= read -r linie
 do
    # Filtrăm liniile care nu îndeplinesc condițiile pentru propoziție corectă
    linii_corecte=$(echo "$linie" | grep -e "^[A-Z][a-zA-Z0-9].*[\.!?]$" | grep -v ", și" | grep -e "$caracter")
    # Verificăm dacă linia a fost filtrată și respectă condiția finală a propoziției corecte
   if [ -n "$linii_corecte" ]
     then
        ultim_ch="${linii_corecte: -1}" #extrag ultimul caracter
        if [ "$ultim_ch" = "?" ] || [ "$ultim_ch" = "!" ] || [ "$ultim_ch" = "." ]
         then
            ((count++))
        fi
    fi
done

# Afișăm numărul de propoziții corecte
echo "$count"

# IELS2001
A respository for tracking the changes and contributions made to the IELS2001 project spring 2024. 

## Hva betyr ordene? 

- add: forteller Git "jeg har endret dette"
- commit: "låser" endringene dine med en besrkivelse
- push: dytter endringene dine til en remote, som for eksempel GitHub, slik at kodebasen er lik for alle på prosjektet
- pull: drar nye endringer fra en remote

## Hvordan gjøre en commit?

først bruk kommandoen "git status" for å sjekke om du er på riktig branch. før du committer må du legge til filene du vil committe ved å bruke kommandoen "git add[filnavn]".
eksempel på kommando: git commit -m "update the README.md with link to contributing guide"
Push dermed endringene til GitHub ved å bruke kommandoen "git push origin main". Da er alt ordnet! Andre vil da kunne se endringene ved å bruke kommandoen "git pull".

For mer info, les mer her: https://github.com/git-guides/git-commit
(eksempelet er via terminalen ved bruk av Git, som må konfigureres om man ikke har gjort dette enda) 

## Hvor ofte skal man gjøre en commit? 

Så ofte som mulig. Her er det best å få oppdatert den felles kodebasen slik at ingen i prosjektet faller bak den nyeste versjonen og har utdatert kode som kan skape konklikter når man senere merger koden.

## Har du gjort noe feil?

Bruk kommandoen "git revert".

## Hvordan gjøre en versjonsendring? 

- små endringer: endrer på de siste nummerene, for eksempel v2.11.3 endres til v2.11.4
- større endringer: endrer på de midterste nummerene, og setter siste nummer til 0, for eksempel v2.11.4 blir v2.12.0
- endre versjon: endrer de første nummerene, setter resten til 0, for eksempel v2.12.0 blir til v3.00.0

For mer info, les her: https://docs.github.com/en/repositories/releasing-projects-on-github/managing-releases-in-a-repository

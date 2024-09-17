# Introduzione

N-Body simulator è un programma in C per simulare il problema degli n-corpi, che si prepone l'obiettivo 
di illustrare l'andamento di corpi (corpi celesti nel nostro caso) che sono soggetti alla forza di gravitazione
universale.

Vengono quindi presentate diverse simulazioni, due seriali usando sia l'algoritmo brute force che quello di 
Barnes-Hut, e 2 implementazioni parallele degli stessi, usando le librerie MPI e Pthread.

Ho quindi aggiunto un renderer per illustrare l'esito delle simulazioni.



# Come avviare le simulazioni

Il progetto è sviluppato su Linux, quindi non assicuro il funzionamento su altre piattaforme visto l'utilizzo
di librerie multi-thread.

Per compilare è necessario inserirsi nella directory specifica dell'implementazione che si vuole testare.
In seguito è sufficiente eseguire il comando:

```
make all
```

Dopo aver compilato il programma è necessario passare all'eseguibile un paio di argomenti:

- "-S" : il nome della simulazione (si veda dopo)
- "-t" : il numero di istanti temporali per cui si vuole simulare

Per semplicità di esecuzione sono fornite alcune simulazioni "note". La mia simulazione di riferimento è
quella del sistema Sole-Terra, osservata per un numero di istanti t pari a 10,000,000. Con questi argomenti
viene prodotta una clip di 9 secondi.

Altre simulazioni note che possono essere osservate sono "square" e  "triangle".

Per eseguire quindi la mia simulazione di riferimento è sufficiente chiamare l'eseguibile nel seguente modo:

```
./main -t 10000000 -S earth_sun
```

Il programma si occuperà di salvare i dati della simulazione in automatico nel suo rispettivo "data.csv"

## Osservare la simulazione

Per osservare la simulazione tramite il renderer è sufficiente: 

- Modificare la variabile "file_name" presente nel file "simulation.ipynb"
- Runnare tutti gli script

Dopo che il programma ha terminato la sua esecuzione, viene prodotta una clip "animation.mp4" della durata 
variabile.

# Funzionamento

L'implementazione del programma è relativamente semplice. Viene adottata la stessa strategia per ogni algoritmo.
L'inizializzazione dei dati viene effettuata secondo i parametri passati come argomento al programma. Viene 
quindi istanziata una lista di particelle, ed in base all'algoritmo adottato le operazioni procedono come segue:

- Brute Force: 

Per ogni particella viene calcolata la forza gravitazionale che intercorre rispetto ad ogni altra particella del sistema.

Viene calcolata l'accelerazione prodotta da tale spinta gravitazionale,e quindi la velocità e la posizione
relative all'istate successivo della simulazione.


- Barnes-Hut:

Viene costruito l'albero quaternario che rappresenta la distribuzione delle particelle nello spazio.
All'interno dell'albero sono presenti diversi dati necessari per le approssimazioni del calcolo delle forze.

Utilizzando l'albero cosi costruito, per ogni particella si calcola la forza totale del sottospazio esercitata
su di essa, secondo un certo parametro configurabile "THETA".

Una volta calcolata la forza totale, si calcolano velocità e posizione delle particelle per l'istante successivo


Tutti questi passaggi vengono ovviamente ripetuti per una totalità di "t" volte.

Per comprendere meglio il funzionamento dell'algoritmo di Barnes-Hut  si consiglia di dare un'occhiata al 
seguente link:

- http://arborjs.org/docs/barnes-hut



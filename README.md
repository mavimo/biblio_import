# Importare dati in Drupal

Spesso capita di dover scrivere procedure di importazione per caricare informazioni presenti da contesti precedenti (durante la migrazione da applicativi in dismisione) o da data source esterni. Fortunatamente Drupal mette a disposizione una suite di funzionalità racchiuse nel modulo [Migrate](https://www.drupal.org/project/migrate).

Questo modulo è stato sviluppato inizialmente da [Cyrve](http://cyrve.com/) società specializzata in data migration per Drupal, e successivamente acquistata da [Acquia](https://www.acquia.com/) che attraverso Mike Ryan ne è l'attuale maintainer. Il modulo è stato utilizzato per migrare grosse quantità di dati in diversi progetti, quali *GenomeWeb* e *The Economist*; per maggiori informazioni vi rimandiamo alla documentazione linkata sulla pagina del progetto.

Il modulo migrate è stato portato a [Drupal 8](https://groups.drupal.org/imp) ed è diventato il meccanismo di base per il processo di upgrade da versioni precedenti di Drupal verso l'ultima versione in fase di rilascio.

In questa guida affronteremo i temi principali relativi alla logica di funzionamento del modulo Migrate e proveremo a scrivere un breve modulo che ci permette di importare il CSV di una ipotetica biblioteca. Questa biblioteca è organizzata per avere i vari libri riposti in stanze, ogni stanza contiene più scaffali, ognuno dei quali ha diversi ripiani e per ogni ripiano sono presenti diversi libri.

## I concetti base

Migrate basa il suo funzionamento sullo stream di informazioni da una sorgente (tipicamente una fonte di dati, quale un CSV, un database, file JSON, webservice, ...) ad una destinazione (tipicamente un entità Drupal come un nodo, un termine di tassonomia, un utente, ..).
Durante il passaggio dei dati tra questi due elementi le informazioni possono essere manipolate, arricchite, ... questo permette la massima flessibilità del processo stesso.

Le *sorgenti* e *destinazioni* dei dati sono implementate attraverso delle classi PHP che -attraverso dei parametri passati al costruttore- definiscono le impostazioni delle stesse.

Le classi principali che troviamo come **sorgenti** sono:

 * ```MigrateSourceCSV```: Utilizzato per sorgenti di dati che sono strutturate in file CSV (o ricondicibili a tali, come ad esempio un foglio di Excel).
 * ```MigrateSourceJSON```: Da usare quando la sorgente di dati è un file JSON.
 * ```MigrateSourceMongoDB```:  Da usare per sorgenti dati presenti sul database documentale MongoDB.
 * ```MigrateSourceSQL```: Da usare per sorgenti dati presenti su database SQL based (tipicamente MySQL / PostgreSQL).
 * ```MigrateSourceXML```:  Da usare quando la sorgente di dati è un file XML.
 
Mentre le pricipali presenti come **destinazione** sono:

 * ```MigrateDestinationEntity```:  costruisce un Entity generica in Drupal, classe tipicalmente estesa per l'implementazione di entità più specifiche, quali quelle di seguito riportate.
 * ```MigrateDestinationComment```: costruisce un oggetto Comment in Drupal. 
 * ```MigrateDestinationFile```:  costruisce un File in Drupal.
 * ```MigrateDestinationNode```:  costruisce un oggetto Node in Drupal.
 * ```MigrateDestinationTerm```:  costruisce un Taxonomy term in Drupal.
 * ```MigrateDestinationUser```:  costruisce un User in Drupal.

Migrate mantiene una relazione tra una determinata sorgente di dati e la sua destinazione, in modo che in caso di una modifica alla sorgente sia possibile effettuare un aggiornamento dei dati migrati consentendo quindi di aggiornare anche i dati importati in Drupal. Questo viene svolto dalla classe di **mapping**; la classe tipicamente utilizzata per questo tipo di operazioni è la classe ```MigrateSQLMap``` che mantiene questa correlazione scrivendo i dati all'interno di una tabella sullo schema SQL dell'istanza di Drupal in cui vengono importati i dati.

Il meccanismo che definisce il passaggio dei dati dalla sorgente alla destinazione, mantenendo il collegamento tra le stesse attraverso un mapping e che consente la manipolazione dei dati nel passaggio è definito come **Migration**.

Da questa brevissima introduziona abbiamo già identificato un subset specifico di componenti che sono coinvolti nella migrazione e sono:

 * La sorgente dati (spesso definita anche come ```DataSource```)
 * La destinazione dei dati (detta anche ```DestinationSoruce```)
 * Il meccanismo di mapping
 * Il meccanismo di migrazione dati (la ```Migration```)

### Data Source

Le sorgenti dati sono appunto il sistema che consente di leggere i dati di partenza da una sorgente esterna. Tipicamente la sorgente dati è costituita da una lista di elementi (```MigrateList```) ognuno dei quali costituisce una riga di dati da migrare (```MigrateItem```).

Prendiamo ad esempio una sorgente definita come *SQL Source*, avremo una query SQL che riporta l'elenco degli oggetti da migrare, questa query restituisce l'elenco degli elementi mentre ogni singola riga dei risultati della query è un item della migrazione (in realtà vedremo in seguito che questa è vero, ma potrebbe anche essere solo un'approssimazione).


### Destination Source

### Mapping

### Migration
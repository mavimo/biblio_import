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

#### SQL Source

Qualora la nostra sorgente fosse un database, quasi sicuramente ci troveremo con l'esigenza di mappare la sorgente dei dati con una query (ricordiamoci, in prima battuta dobbiamo riportare ad ogni riga un elemento da migrare).

Una volta individuata la query che ci consente di avere la nostra sorgende di dati dobbiamo "trasformarla" nell'equivalente della query definita usando la ```SelectQueryInterface``` di Drupal, quindi ad esempio:

```sql
SELECT fname, lname, birthday FROM myapp_user WHERE birthday < NOW();
````

Verrà convertita in:

```php
$query = db_select('myapp_user', 'u')
             ->fields('u', array('fname', 'lname', 'birthday'));
$query->condition('u.birtday', 'NOW()', '>');

$source = new MigrateSourceSQL($query);
```

Una volta costruita la query dovremmo passare questa informazione alla classe ```MigrateSourceSQL``` che costruirà la sorgente di dati usata da Migrate per effettuare la migrazione dei dati.

La signature completa del costruttore della classe riporta:

```php
MigrateSourceSQL(
  SelectQueryInterface $query,
  array $fields = array(),
  SelectQueryInterface $count_query = NULL,
  array $options = array()
)
```

Dove appunto vediamo che il primo (e unico) parametro obbligatorio è appunto la query che fa da sorgente nella nostra migrazione.

#### CSV Source


Qualora la nostra sorgente fosse un file CSV dovremmo passare questa informazione alla classe ```MigrateSourceCSV``` che costruirà la sorgente di dati usata da Migrate per effettuare la migrazione dei dati.

La signature completa del costruttore della classe riporta:

```php
MigrateSourceCSV(
  $path,
  array $csvcolumns = array(),
  array $options = array(),
  array $fields = array()
)
```

Dove appunto vediamo che il primo (e unico) parametro obbligatorio è il percorso del file contenente la nostra sorgente dati. Il secondo parametro, invece, riporta l'elenco delle colonne presente nel CSV. Il sistema, nel caso in cui non sia definito, cercherà di individuare i campi presenti nel file e individuare quindi le colonne, ma per comodità è sempre preferibile esplicitare le colonne che ne fanno parte. Il terzo parametro, invece, specifica le opzioni di parsing del CSV, quindi il separatore dei campi, l'enclosure (come vengono "racchiusi" i campi) ed il carattere di escaping ed è anche è utilizzato per indicare la presenza di una riga di intestazione.

Facciamo l'esempio di una sorgente contenente appunto nome, cognome e data di nascita, in un file CSV (con nome ```app_users.csv``` posizionato nella cartella ```data``` del nostro modulo) come di seguito riportato:

```csv
fname;lname;brithday
"Marco";"Moscaritolo";1983-04-22
"Mario";"Rossi";1984-09-03
```

Nel nostro caso avremo:

```php
$path = drupal_get_path('module', 'biblio_import') . '/data/app_users.csv';

$columns = array(
  array('fname', 'User: First name'),
  array('lname', 'User: Last name'),
  array('birthday', 'User: Birthday'),
);

$options = array(
  'delimiter' => ';',
  'enclosure' => '"',
  'escape' => '\\',
  'header_rows' => 1,
);

$source = new MigrateSourceCSV($path, $columns, $options);
```

dove nelle colonne è specificato come primo valore dell'array il nome della colonna, mentre nel secondo la descrizione visibile nella UI di migrate.

### Destination Source

Se il *Data Source* ci serviva a definire da dove prelevare i dati da migrare, con il *Destination Source* specificheremo dove le informazioni vanno a confluire all'interno di Drupal. Nella quasi totalità dei casi le destinazioni dei nostri dati sono delle entità Drupal (nodi, utenti, termini di tassonomia, ...) pertanto -come abbiamo visto in precedenza- esistono le classi di destinazione già customizzate per le principali entity di base di Drupal, con la possibilità di creare destinazioni custom ulteriori.
Vediamo ad esempio le tre principali classi di destination:

#### Node

Prendiamo ad esempio la classe ```MigrateDestinationNode``` che permette di definire come destinazione una entity di tipo ```Node```, questa classe accetta come parametro il node type dell'entità che deve essere costituita (il bundle), per istanziare una destinazione di nodo, quindi è necessario costruirla specificando questo parametro, ad esempio:

```php
$destination = new MigrateDestinationNode('biblio_doc');
```
Questo indica al sistema che per ogni riga sorgente avremo una entity di tipo node di tipo *biblio_doc* creata.

#### User

Allo stesso modo la classe ```MigrateDestinationUser``` definisce che l'entity che viene creata è di tipo utente; in questo caso non è necessario specificare ulteriori parametri:

```php
$destination = new MigrateDestinationUser();
```

#### Termine della tassonomia

Prendiamo ad esempio la classe ```MigrateDestinationTerm``` che permette di definire come destinazione una entity di tipo ```TaxonomyTerm```, questa classe accetta come parametro il machine name del vocabolario a cui il termini creato deve appartenere, quindi ad esempio:

```php
$destination = new MigrateDestinationTerm('biblio_book_type');
```

Vi è un secondo parametro usato per la gestione dei duplicati, in caso di necessità vi rimandiamo alla documentazione per approfondire i valiri delle opzioni.

### Mapping

Come abbiamo detto precedentemente Drupal mantiene un collegamento tra la sorgente e la destinazione (l'entity di Drupal creata) della migrazione, per fare questo viene usato un mapping, mapping che solitamente viene costituito usando la classe di mapping ```MigrateSQLMap```. Questa classe ha la seguente firma del costruttore:

```php
MigrateSQLMap(
  $machine_name,
  array $source_key,
  array $destination_key,
  $connection_key = 'default',
  $options = array()
)
```
Dove il primo parametro indica il nome identificativo del mapping, solitamente questo valore corrisponde con in nome della migrazione corrente, ma può essere variato in alcune situazioni, ad esempio per avere migrazioni differenti ma con un unica tabella di mapping. Solitamente il valore usato per definire il machine name è la proprietà ```machineName``` della migrazione corrente.

Il secondo parametro (```$source_key```) identifica le chiavi delle sorgenti usate come chiavi identificative della l'elemento della sorgente. A scopo di esempio, supponiamo che l'elenco di utenti usato come esempio precedentemente non consenta l'omonimia, per constraddistinguere in maniera univoca un utente è necessario la coppia *fname/lname*, pertanto la nostra chiave sorgente sarà definita come costituita da queste due chiavi. Per ognuna delle chiavi poi dovranno essere definiti gli attributi della colonna SQL necessari ad archiviare questo dato, nel nostro esempio avremo qualcosa di simile a:
```php
$source_key = array(
  'fname' => array(
    'type' => 'varchar',
    'length' => 255,
    'not null' => TRUE,
    'description' => 'User first name'
  ),
  'lname' => array(
    'type' => 'varchar',
    'length' => 255,
    'not null' => TRUE,
    'description' => 'User last name'
  ),
);
```

La stessa cosa viene effettuata per la ```$destination_key``` con la differenza che in questo caso andremo ad usare la chiave primaria dell'entity creata durante la migrazione, ad esempio per gli utenti andremo ad usare l'uid. Per semplificare la definizione tutte le destination source definiscono un metodo statico ```getKeySchema``` che serve a restituire appunto la definizione della destination key, quindi per l'utente avremo:

```php
$destination_key = MigrateDestinationUser::getKeySchema();
```

Di coneguenza la nostra migration map sarà simile a:

```php
$map = new MigrateSQLMap(
  $this->machineName,
  array(
    'fname' => array(
      'type' => 'varchar',
      'length' => 255,
      'not null' => TRUE,
      'description' => 'User first name'
    ),
    'lname' => array(
      'type' => 'varchar',
      'length' => 255,
      'not null' => TRUE,
      'description' => 'User last name'
    ),
  ),
  MigrateDestinationUser::getKeySchema()
);
```


### Migration

Arriviamo ora al collante di tutto ciò che abbiamo visto fino ad ora, la classe di migrazione. Questa classe tipicamente estende la classe base di migration ```Migration``` e implementa nel costruttore tutte le configurazioni necessarie, a solo scopo esemplificativo:

```php
class BiblioUserMigrate extends Migrate {
  public function __construct($arguments) {
    parent::__construct($arguments);

    // Mapping =================================================================
    $this->map = new MigrateSQLMap(
      $this->machineName,
      array(
        'fname' => array(
          'type' => 'varchar',
          'length' => 255,
          'not null' => TRUE,
          'description' => 'User first name'
        ),
        'lname' => array(
          'type' => 'varchar',
          'length' => 255,
          'not null' => TRUE,
          'description' => 'User last name'
        ),
      ),
      MigrateDestinationUser::getKeySchema()
    );

    // Source ==================================================================
    $path = drupal_get_path('module', 'biblio_import') . '/data/app_users.csv';

    $columns = array(
      array('fname', 'User: First name'),
      array('lname', 'User: Last name'),
      array('birthday', 'User: Birthday'),
    );

    $options = array(
      'delimiter' => ';',
      'enclosure' => '"',
      'escape' => '\\',
      'header_rows' => 1,
    );

    $this->source = new MigrateSourceCSV($path, $columns, $options);

    // Destination =============================================================
    $this->destination = new MigrateDestinationUser();
  }
}
```

dove come possiamo vedere abbiamo usato le classi di source, destination e mapping viste in precedenza, assegnandone il valore a delle proprietà della classi migrate, in particolare alle proprietà ```map```, ```source``` e ```destination```.

#### Modificare i dati in ingresso

Qualche volta i dati in ingresso hanno necessità di essere modificati o di essere estesi, ad esempio per creare dati aggiuntivi. Nell'esempio che abbiamo preso in considerazione fino ad ora nell'importazione dell'utente -ad esempio- non abbiamo un attributo definito come username, che vogliamo corrisponda con nome e cogonome, in minuscolo, separati da un punto. Per fare questo possiamo sfruttare il metodo ```prepareRow``` della migrazione. Questo metodo permette di alterare i dati in ingresso per ottenere le informazioni che riteniamo necessarie, ad esempio:

```php
class BiblioUserMigrate extends Migrate {
  public function __construct($arguments) {
    // ...
    $this->addFieldMapping('username', 'generated_username');
  }

  public function prepareRow(&$row) {
    $row->generated_username = strtolower($row->fname . '.' . $row->lname);
  }
}
```

La stessa funzione può anche essere utilizzata per evitare che un determinato record venga migrato; per fare questo è sufficiente che la funzione ritorni false. Nel nostro esempio vogliamo escludere l'importazione dei dati dove la lunghezza del nome o del cognome è minore di 3 caratteri perché probabilmente si tratta di un dato fasullo; la nostra implementazione diventerebbe:

```php
class BiblioUserMigrate extends Migrate {
  // ...

  public function prepareRow(&$row) {
    if (strlen($row->fname) < 3 || strlen($row->lname) < 3) {
      return FALSE;
    }

    // ...
  }
}
```

### Field mapping

Con le informazioni minime viste in precedenza la nostra migrazione è effetivamente utilizzabile, ma manca ancora un ultimo step molto importante, ovvero il mapping delle informazioni estratte nella sorgente con quelle della destinazione. Questa operazione viene effettuata usando il metodo ```addFiedlMapping``` della classe di migrazione.

Questo metodo ha la seguente firma:

```php
addFieldMapping(
  $destination_field,
  $source_field = NULL,
  $warn_on_override = TRUE
);
```

Dove il primo valore indica il nome del field di destinazione, ovvero la proprietà dell'entità di destinazione in cui le informazioni verrano inserite. Queste proprietà possono essere proprietà tipiche dell'entity di destinazione (ad esempio username o password per l'utente) o i machine name dei valori dei field. Supponendo che abbiamo inserito sull'utente in drupal tre field, due testuali e uno di tipo date, i cui machine name saranno:

 * field_user_fname
 * field_user_lname
 * field_user_birthday

Il secondo valore indica il nome nella sorgente, ad esempio il nome della colonna nelle migrazioni da CSV come sorgente o il nome del campo della tabella risultante come query in caso di sorgenti SQL.

L'ultimo valore in input indica se scatenare un warn nel caso in cui la migrazione effettui il mapping due volte della stessa property.

Il field mapping della migrazione sopra definita risulterà quindi simile a:

```php
$this->addFieldMapping('field_user_fname', 'fname');
$this->addFieldMapping('field_user_lname', 'lname');
$this->addFieldMapping('field_user_birthday', 'birthday');
```

dove appunto stiamo inserendo i valori delle colonne dei campi CSV dentro i field dell'entity di Drupal.

#### Valori di default

Come potete ben immaginare non sempre tutti i campi delle entità di destinazione sono presenti nei dati di partenza, quindi potrebbe essere necessario predisporre dei valori predefiniti per alcuni di essi, ad esempio per gli utenti potremmo voler specificare che questi utenti devono essere attivi e che la data di creazione e modifica non è la data corrente, ma il 1 gennaio 2015.

Per fare questo è possibile mappare, usando la funzione già vista in precedenza, solamente il campi di destinazione e utilizzare poi il metodo ```defaultValue```, quindi nel nostro caso avremo:

```php
$this->addFieldMapping('status')->defaultValue(1);
$this->addFieldMapping('created')->defaultValue(strtotime('2015-01-01'));
```

#### Valori multipli

Nel momento in cui abbiamo necessità di avere dei valori multipli associati ad un singolo field, ad esempio perché all'utente abbiamo assegnato assegnato un campo di tipo testo multiplo (chiamato ```field_user_notes```) in cui vengono inserite alcune note. Nel nostro CSV di partenza, essendo che per ogni elemento migrato (quindi ogni utente) dobbiamo avere una sola riga, avremo le diverse note concatenate con un carattere separatore, ad esempio il carattere ```|```. Il nostro fil CSV diventerebbe qui:

```csv
fname;lname;brithday;notes
"Marco";"Moscaritolo";1983-04-22;"Prima nota|Seconda nota"
"Mario";"Rossi";1984-09-03;"Terza nota|Quarta nota|Quinta nota"
```

Per far si che questo venga interpretato correttamente durante la migrazione useremo il metodo ```separator```, specificando come argomento il carattere da usare -appunto- come separatore:

```php
$this->addFieldMapping('field_user_notes', 'notes')->separator('|');
```

#### Callbacks

Cosa succede invece quando il valore di una proprietà non è corrispondente al valore sorgente, ma dobbiamo procedere con una modifica (ad esempio dobbiamo convertire il formato della data)? Ci viene in aiuto l'utilizzo delle callback che servono appunto a processare un dato in ingresso prima che questo venga preso in considerazione dalla migrazione, ma senza modificare il dato della sorgente, ovvero mantenendo l'informazione originale come dato di sorgente.

Nel nostro esempio precedente, per la data avremo:

```php
$this->addFieldMapping('field_user_birthday', 'birthday')->callbacks('strtotime');
```

dove stiamo dicendo che prima di assegnare alla proprietà ```field_user_birthday``` dell'utente il valore presente nel campo ```birthday``` della sorgente, andremo a passare questo valore attraverso la funzione ```strtotime``` di PHP, ovvero faremo un operazione simile a:

```php
$destination->field_user_birthday = strtotime($source->birthday);
```

Qualora avessimo necessità di svolgere operazioni più complesse, potremmo dichiarare all'interno della nostra classe di migrazione un metodo pubblico e usare questo come callback, ad esempio:

```php
class BiblioUserMigrate extends Migrate {
  public function __construct($arguments) {
    // ...
    $this->addFieldMapping('field_user_birthday', 'birthday')
      ->callbacks(array($this, 'convertBirthday'));
  }

  public function convertBirthday($data) {
    // Faccio quello che serve...
    return strtotime($data);
  }
}
```

#### Dipendenze tra migrazioni

Spesso capita che determinate migrazioni dipendano da altri informazioni che devono essere migrate, ad esempio potremmo voler taggare i nostri utenti con dei tag che sono a loro volta importati da un altra migrazione, ad esempio se consideriamo i nostri utenti della bilbioteca potremmo avere diverse categorie di generi lettari e vorremo taggare gli utenti con i generi letterari che preferiscono.

La sorgente delle categorie letterarie, sempre da CSV, risulta quindi essere:

```csv
category
"Romanzo"
"Sci-fi"
"Saggio"
"Fantasy"
```

E la corrispettiva migrazione, che importa questi tag all'interno del vocabolario ```book_category```:

```php
class BookCategoryMigrate extends Migrate {
  public function __construct($arguments) {
    parent::__construct($arguments);

    // Mapping =================================================================
    $this->map = new MigrateSQLMap(
      $this->machineName,
      array(
        'category' => array(
          'type' => 'varchar',
          'length' => 255,
          'not null' => TRUE,
          'description' => 'Book category'
        ),
      ),
      MigrateDestinationTerm::getKeySchema()
    );

    // Source ==================================================================
    $path = drupal_get_path('module', 'biblio_import') . '/data/book_category.csv';

    $columns = array(
      array('category', 'Book category'),
    );

    $options = array(
      'delimiter' => ';',
      'enclosure' => '"',
      'escape' => '\\',
      'header_rows' => 1,
    );

    $this->source = new MigrateSourceCSV($path, $columns, $options);

    // Destination =============================================================
    $this->destination = new MigrateDestinationTerm('book_category');

    // Fileds mapping ==========================================================
    $this->addFieldMapping('name', 'category');

    // ...
  }
}
```

La migrazione procede quindi creando dei termini di tassonomia, mappando come chiave della sorgente il nome del termine stesso.

Nella nostra sorgente di dati dobbiamo quindi avere informazioni relative alla categorie di interesse dell'utente, quindi il nostro CSV diventerà:


```csv
fname;lname;brithday;notes;preferences
"Marco";"Moscaritolo";1983-04-22;"Prima nota|Seconda nota";"Sci-Fi|Fantasy"
"Mario";"Rossi";1984-09-03;"Terza nota|Quarta nota|Quinta nota";"Sci-Fi|Romanzo"
```

E di conseguenza la migrazione degli utenti, ipotizzando che il nome del campo associato all'utente sia ```field_user_categories``` avremo:

```php
$this->addFieldMapping('field_user_categories', 'categories')
  ->separator('|')
  ->sourceMigration('BookCategory');
```

come vediamo stiamo usando nuovamente il metodo ```separator``` poiché abbiamo più di un elemento nel nostro campo, ma la parte interessante è il metodo ```sourceMigration``, che indica che migrazione usare per "convertire" il nostro valore di partenza con il valore identificativo dell'entità di destinazione della migrazione dipendente. Nel caso riprotato pocanzi la migrazione ```BookCategoryMigration``` ha migrato i dati del CSV in termini della tassonomia, salvando nella tabella di mapping la chiave della sorgente (il nome del termine) e la chiave di destinazione (l'ID del termine, il TID). Il source migration utilizza i dati salvati nella tabella di mapping per convertire il valore in ingresso (che sarà la chiave della sorgente) nella corrispettiva chiave della destinazione. questo consente di avere i TID corretti che sono quelli che vengono poi realmente referenziati all'interno della destinazione della migrazione utente.

## Dichiarazione delle migrazioni

Una volta costruite le migrazioni come visto fino ad ora dobbiamo "informare" Drupal dell'esistenza di queste migrazioni. Per fare questo dobbiamo creare un modulo che fornisca queste informazioni. Per fare questo dovremo creare un modulo (con i classici file ```info``` e ```module```). Nel file ```info``` avremo:

```ini
name = "Biblio import"
description = "Import biblio informations"
package = "Biblio"
core = 7.x

dependencies[] = migrate

files[] = migration/biblio_category.inc
files[] = migration/biblio_user.inc
files[] = migration/biblio_item.inc
```

in cui, oltre alle classiche informazioni sul nome, descrizione, package e versione di Drupal, abbiamo definito una dipendenza verso il modulo ```migrate``` (che si occupa di effettuare le migrazioni) e dell'esistenza di tre file contenenti le classi di migrazioni.

Il file ```module``` non necessità di particolari dichiarazioni, quindi avremo:

```php
<?php

/**
 * @file
 * Import biblio informations in Drupal
 */
```

Oltre alla definizione del modulo creeremo il file ```NOME_MODULO.migrate.inc``` in cui andremo a definire l'implementazione dell'```hook_migrate_api``` che riporta effettivamente quali sono le migrazioni che il nostro modulo implementa, quindi:

```php
<?php
/**
 * @file
 * Our own hook implementation.
 */

/**
 * Implements hook_migrate_api().
 */
function biblio_import_migrate_api() {
  $api = array(
    'api' => MIGRATE_API_VERSION,
    'groups' => array(
      'biblio' => array(
        'title' => t('Biblio'),
      ),
    ),
    'migrations' => array(
      'BiblioTerm' => array(
        'class_name' => 'BiblioTermMigration',
        'group_name' => 'biblio',
      ),
      'BiblioUser' => array(
        'class_name' => 'BiblioUserMigration',
        'group_name' => 'biblio',
      ),
      'BiblioItem' => array(
        'class_name' => 'BiblioItemMigration',
        'group_name' => 'biblio',
      ),
    ),
  );

  return $api;
}
```

dove stiamo indicando che esiste un gruppo di migrazione (con machien name ```biblio```) la cui label sarà ```Biblio```, e tre migrazioni in cui indichiamo per ogni machine name quale è la classe di migrazione che definisce la migrazione e a quale gruppo questa migrazione appartiene.

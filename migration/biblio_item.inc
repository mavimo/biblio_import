<?php
/**
 * @file
 * Import biblio items.
 */
class BiblioItemMigration extends Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->team = array(
      new MigrateTeamMember('Marco Vito Moscaritolo', 'marco@mavimo.org', t('Developer')),
    );

    $this->description = t('');

    $this->map = new MigrateSQLMap($this->machineName,
      [
        'room' => [
          'type' => 'varchar',
          'length' => 255,
          'not null' => TRUE,
          'description' => 'Room',
        ],
      ],
      MigrateDestinationNode::getKeySchema()
    );

    $columns = [
      ['room',    'room'],
      ['closet',  'closet'],
      ['shelf',   'shelf'],
      ['item_id', 'item_id'],
    ];

    $options = [
      'header_rows' => 1,
      'delimiter' => ',',
      'enclosure' => '"',
    ];

    $source_file = drupal_get_path('module', 'biblio_import') . '/data/items.csv';
    $this->source = new MigrateSourceCSV($source_file, $columns);
    $this->destination = new MigrateDestinationNode('biblio_book');

    $this->addFieldMapping('field_biblio_room',   'room')->description(t(''));
    $this->addFieldMapping('field_biblio_closet', 'closet')->description(t(''));
    $this->addFieldMapping('field_biblio_shelf',  'shelf')->description(t(''));
    $this->addFieldMapping('field_biblio_id',     'item_id')->description(t(''));

    $this->addUnmigratedDestinations([
      'title',
      'uid',
      'created',
      'changed',
      'status',
      'promote',
      'sticky',
      'revision',
      'log',
      'language',
      'tnid',
      'translate',
      'revision_uid',
      'is_new',
      'path',
      'comment',
      'field_biblio_room:language',
      'field_biblio_id:language',
    ]);
  }
}

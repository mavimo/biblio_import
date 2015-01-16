<?php
/**
 * @file
 * Import biblio items.
 */
class BiblioCategoryMigration extends Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->team = array(
      new MigrateTeamMember('Marco Vito Moscaritolo', 'marco@mavimo.org', t('Developer')),
    );

    $this->description = t('');

    $this->map = new MigrateSQLMap($this->machineName,
      [
        'category' => [
          'type' => 'varchar',
          'length' => 255,
          'not null' => TRUE,
          'description' => 'Room',
        ],
      ],
      MigrateDestinationTerm::getKeySchema()
    );

    $columns = [
      ['category',    'Category'],
    ];

    $options = [
      'header_rows' => 1,
      'delimiter' => ',',
      'enclosure' => '"',
    ];

    $source_file = drupal_get_path('module', 'biblio_import') . '/data/categories.csv';
    $this->source = new MigrateSourceCSV($source_file, $columns);
    $this->destination = new MigrateDestinationTerm('biblio_category');

    $this->addFieldMapping('name', 'category');
  }
}

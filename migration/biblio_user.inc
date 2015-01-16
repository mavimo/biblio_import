<?php
/**
 * @file
 * Import biblio items.
 */
class BiblioUserMigrate extends Migrate {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->team = array(
      new MigrateTeamMember('Marco Vito Moscaritolo', 'marco@mavimo.org', t('Developer')),
    );

    $this->description = t('');

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
    $path = drupal_get_path('module', 'biblio_import') . '/data/users.csv';

    $columns = array(
      array('fname', 'User: First name'),
      array('lname', 'User: Last name'),
      array('birthday', 'User: Birthday'),
      array('notes', 'User: Notes'),
      array('categories', 'User: Categories'),
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

    // Field mapping ===========================================================
    $this->addFieldMapping('field_user_fname', 'fname');
    $this->addFieldMapping('field_user_lname', 'lname');
    $this->addFieldMapping('field_user_notes', 'notes')->separator('|');
    $this->addFieldMapping('field_user_birthday', 'birthday')->callbacks('strtotime');
    $this->addFieldMapping('field_user_categories', 'categories')
      ->separator('|')
      ->sourceMigration('BiblioCategory');

    $this->addFieldMapping('status')->defaultValue(1);
    $this->addFieldMapping('created')->defaultValue(strtotime('2015-01-01'));
  }

  public function prepareRow(&$row) {
    if (strlen($row->fname) < 3 || strlen($row->lname) < 3) {
      return FALSE;
    }

    $row->generated_username = strtolower($row->fname . '.' . $row->lname);
  }
}

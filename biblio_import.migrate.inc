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
      'BiblioItem' => array(
        'class_name' => 'BiblioItemMigration',
        'group_name' => 'biblio',
      ),
      'BiblioUser' => array(
        'class_name' => 'BiblioUserMigration',
        'group_name' => 'biblio',
      ),
      'BiblioCategory' => array(
        'class_name' => 'BiblioCategoryMigration',
        'group_name' => 'biblio',
      ),
    ),
  );

  return $api;
}

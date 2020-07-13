<?php
/**
 * Provide an extended class for multi inserting of Chado.
 * This class may later on be used for further needs.
 *
 * Specifically tihs class provides select, insert, update and delete.
 *
 */

class ChadoRecordList extends ChadoRecord {

  /**
   * InsertPartitionLimit used to batch inserts.
   *
   * @var int
   */
  protected $insert_partition_limit = 10000;

  /**
   * Retrieves the insert parition limit value used to batch inserts.
   *
   * @return int
   *   The value of the insert parition limit.
   */
  public function getInsertPartitionLimit() {
    return $this->insert_partition_limit;
  }

  /**
   * Set the insert parition limit value used to batch inserts.
   *
   * @param int $value
   *   The new insert partition limit.
   */
  public function setInsertPartitionLimit($value) {
    $this->insert_partition_limit = $value;
  }

  /**
   * The ChadoRecordList constructor
   *
   * @param string $table_name
   *   The name of the table that the record belongs to.
   * @param array $record_ids
   *  An optional array of record IDs if these records already exist in Chado.
   */
  public function __construct($table_name, array $record_ids = array()) {

    if (!$table_name) {
      $message = t('ChadoRecordList::_construct(). The $table_name argument is required for a ChadoRecord instance.');
      throw new Exception($message);
    }

    // Set the table name and schema.
    $this->table_name = $table_name;
    $this->schema = chado_get_schema($this->table_name);
    if (!$this->schema) {
      $message = t('ChadoRecordList::_construct(). Could not find a matching table schema in Chado for the table: !table.',
        ['!table' => $this->table_name]);
      throw new Exception($message);
    }

    // Chado tables never have more than one column as a primary key so
    // we are good just getting the first element.
    $this->pkey = $this->schema['primary key'][0];

    // Save the column names.
    foreach ($this->schema['fields'] as $column_name => $col_details) {
      $this->column_names[] = $column_name;
    }

    // Get the required columns.
    foreach ($this->schema['fields'] as $column => $col_schema) {
      foreach ($col_schema as $param => $val) {
        if (preg_match('/not null/i', $param) and $col_schema[$param]) {
          $this->required_cols[] = $column;
        }
      }
    }

    if (!empty($record_ids)) {
      try {
        $sql = 'SELECT * FROM {' . $this->table_name . '} WHERE ' . $this->pkey . ' IN (';
        $args = array();
        $placeholders = array();
        $i = 0;
        foreach ($record_ids as $id) {
          $placeholder = ":record_id_$i";
          $placeholders[] = $placeholder;
          $args[$placeholder] = $id;
          $i++;
        }
        $sql .= implode(', ', $placeholders) . ');';
        $results = chado_query($sql, $args)->fetchAllAssoc($this->pkey);

        if (count($results) != count($record_ids)) {
          // Find the missing ids and throw an error.
          $missing_ids = $record_ids;
          foreach ($missing_ids as $idx => $id) {
            if (!empty($results[$id])) {
              unset($missing_ids[$idx]);
            }
          }

          $message = t('ChadoRecordList::_construct(). Could not find record(s) in table, !table, with the given !pkey: !record_id.',
            [
              '!pkey' => $this->pkey,
              '!record_id' => implode(', ', $missing_ids),
              '!table' => $this->table_name,
            ]);
          throw new Exception($message);
        }

        $this->record_ids = $record_ids;
        $this->values = $results;
        $this->missing_required_col = [];
      }
      catch (\Exception $e) {
        $message = t('ChadoRecordList::_construct(). Could not find record(s) in table, !table, with the given !pkey: !record_id. ERROR: !error',
        [
          '!pkey' => $this->pkey,
          '!record_id' => implode(', ', $record_ids),
          '!table' => $this->table_name,
          '!error' => $e->getMessage(),
        ]);
        throw new Exception($message);
      }
    }
  }

  /**
   *  Inserts multiple rows using a db_insert object where
   *  $records is an array of arrays of form array('column' => 'value', ...), array(...)
   *  It also allows batch inserts based on 'partitioning' the records
   *  The current default partition is 10000 so inserts are done every 10000 records
   *  You can modify partition using setInsertPartitionLimit(value)
   *
   *  //Initial sample code was provided here: https://github.com/tripal/tripal/issues/1068
   *
   *  @throws Exception
   */
  public function insertMultiple($records, array $options = array()) {
    /**
     * The for statement might be a little hard to read, please remember the specific order matters for accuracy.
     */

    $partition_limit = $this->getInsertPartitionLimit(); //Number of records to process at a time (we will call this a partition)
    $records_count = count($records);

    if ($records_count < 1) {
      $message = t('ChadoRecord::insertMultiple(). Could not insert a record into the table !table, no records provided.',
        ['!table' => $this->table_name]);
      throw new Exception($message);
    }    


    //Iterate through each record
    $result = NULL;
    for($i = 0; $i < $records_count; $i++) {
      if($i % $partition_limit == 0) { //If partition_limit has been met during iterations
        if($i > 0) { //we want to execute only after the start, not when $i == 0
          $result = $insert->execute();
        }
        $insert = db_insert(chado_get_schema_name('chado') . '.' . $this->getTable(), $options)
          ->fields($this->column_names); //create insert object at start or every partition of records
      }

      $insert->values($records[$i]); //Always add values to the insert object per record

      if($i == ($records_count - 1)) { //cater for if $i never reaches the partition_limit, we need to finish off with an execute command
        $result = $insert->execute();
      }
    }
    return $result;
  } 
  
  /**
   * TODO: Insert a new row with values into protected:values
   */
  public function insertRow($values) {

  }
}
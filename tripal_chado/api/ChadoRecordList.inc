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
   */
  public function setInsertPartitionLimit($value) {
    $this->insert_partition_limit = $value;
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
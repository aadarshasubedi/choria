<?
	include("topinclude.php");
	@$changed = intval($_GET['changed']);
	$table = $_GET['table'];
	@$sort = $_GET['sort'];
	@$reverse = intval($_GET['reverse']);
?>

<? if($changed) echo "<div class=\"changed\">Changed</div>"; ?>
<table border="0">
	
<?
	$sort_sql = "";
	if($sort != "")
		$sort_sql = "order by $sort";
	
	$reverse_sql = "";
	if($reverse)
		$reverse_sql = "desc";
	$query = $Database->query("select * from $table $sort_sql $reverse_sql");
	if(!$query) {
		print_r($Database->errorInfo());
	}
	$results = $query->fetchAll(PDO::FETCH_ASSOC);
	//print_r($results);
	$array_keys = array_keys($results[0]);
	$key_count = count($array_keys);
?>
	<tr class="header">
		<? for($i = 0; $i < $key_count; $i++) {
			$field = $array_keys[$i];
			$reverse_string = "";
			if($sort == $field)
				$reverse_string = "&reverse=" . (1-$reverse);
		?>
		<td><a href="data.php?table=<?=$table?>&sort=<?=$field?><?=$reverse_string?>"><?=$field?></a></td>
		<? } ?>
	</tr>
<?

	foreach($results as $result) {
		$id = $result[$array_keys[0]];
?>
	<tr>
		<td><a href="data_ae.php?id=<?=$id?>&table=<?=$table?>&mode=edit"><?=$id?></a></td>
		<? for($i = 1; $i < $key_count; $i++) { ?>
		<td><?=$result[$array_keys[$i]]?></td>
		<? } ?>
	</tr>
<?
	}
?>
</table>

<?
	include("footer.php");
?>

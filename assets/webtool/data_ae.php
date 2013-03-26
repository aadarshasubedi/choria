<?
	include("topinclude.php");
	$id = intval($_GET["id"]);
	$table = $_GET["table"];
	$mode = $_GET["mode"];
	
	if(isset($_POST["Submit"])) {
		if($mode == "edit") {
			unset($_POST["Submit"]);
			//$keys = array_keys($_POST);
			//$values = array_values($_POST);
			
			$update_array = array();
			foreach($_POST as $name=>$value)
				array_push($update_array,  "$name = \"$value\"");
				
			$sql = "update $table set " . implode(", ", $update_array) . " where ID = $id";
			//echo $sql;
			//exit;
			if(!$Database->query($sql)) {
				print_r($Database->errorInfo());
				exit();
			}
			header("Location: data.php?table=$table&changed=1");
			exit;
		}
	}
	
	$query = $Database->query("select * from $table where ID = $id");
	$result = $query->fetch(PDO::FETCH_ASSOC);
	$keys = array_keys($result);
?>
	<div style="margin-bottom: 10px; font-weight: bold;">
		Edit <?=$table?> stats for <?=$result[$keys[1]]?> (ID=<?=$id?>)
	</div>
	<form name="Form" action="data_ae.php?mode=<?=$mode?>&id=<?=$id?>&table=<?=$table?>" method="post" class="regularform">
	<?
		foreach($result as $name=>$value) { 
			if($name == "ID")
				continue;
	?>
		<label for="name"><?=$name?> <input name="<?=$name?>" type="text" value="<?=$value?>" /></label>
	<?	}	
	?>
		
	<div style="margin-top: 20px">
		<input type="submit" name="Submit" value="Submit" />
	</div>
	</form>

<?
	include("footer.php");
?>
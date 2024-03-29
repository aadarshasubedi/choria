var hot = null;
$(document).ready(function() {
	$(document).ajaxError(function() {
		  $("#message").text("check console");
	});

	querystring = location.search;
	if(querystring == "") {
		querystring = "?table=item";
	}

	// Load spreadsheet
	var container = document.getElementById('sheet');
	data_url = "/data" + querystring;
	$.getJSON(data_url, function(response) {
		var data = response['data'];
		var message = response['message'];
		column_names = response['column_names'];
		references = response['references'];
		children = response['children'];

		// Show messages
		if(message != undefined) {
			$('#message').html(message);
			return;
		}

		// Build dropdown data
		var columns = [];
		transform_data(data, columns);

		// Create spreadsheet
		hot = new Handsontable(container, {
			data: data,
			rowHeaders: true,
			colHeaders: column_names,
			columnSorting: true,
			columns: columns,
			trimDropdown: false,
			manualColumnResize: true,
			wordWrap: false,
			contextMenu: {
				items: {
					"select_column": { name: 'Select Column' },
					"select_row": { name: 'Select Row' },
					"hsep0": "---------",
					"freeze_column": { name: 'Freeze Column' },
					"freeze_row": { name: 'Freeze Row' },
					"hsep1": "---------",
					"undo": {},
					"redo": {},
				},
				callback: function(key, options) {
					if(key === 'select_column') {
						var selected = hot.getSelected();
						hot.selectCell(0, selected[1], hot.countRows()-1, selected[1], false);
					}
					else if(key === 'select_row') {
						var selected = hot.getSelected();
						hot.selectCell(selected[0], 0, selected[0], hot.countCols()-1, false);
					}
					else if(key === 'freeze_column') {
						var selected = hot.getSelected();
						hot.updateSettings({ fixedColumnsLeft: selected[1] });
					}
					else if(key === 'freeze_row') {
						var selected = hot.getSelected();
						hot.updateSettings({ fixedRowsTop: selected[0] });
					}
				},
			},
			cells: function(row, col, prop) {
				var cellProperties = {};
				if(col == 0) {
					cellProperties.readOnly = true;
				}
				else if(column_names[col] == "") {
					cellProperties.editor = false;
					cellProperties.disableVisualSelection = true;
				}

				return cellProperties;
			},
			afterChange: function(changes, source) {
				update_buttons(0);
			},
			afterSelection: function(r, c, r2, c2) {
				$('#delete_id').val(hot.getDataAtCell(r, 0));
			},
		});

		update_buttons(1);
	});

	// load nav links
	$.getJSON("/tables", function(response) {
		tables = response;
		for(i in tables) {
			table = tables[i];
			li_element = document.createElement("li");
			a_element = document.createElement("a");
			$(a_element).attr("href", "/?table=" + table);
			$(a_element).html(table);
			$(li_element).append(a_element);
			$('#tables').append(li_element);
		}
	});

	// Add handlers
	$('#save').click(function() { save(); });
	$('#add').click(function() { add(); });
	$('#delete').click(function() { remove(); });
});

// Function to render related table links
function link_renderer(instance, td, row, col, prop, value, cellProperties) {
	td.innerHTML = value;
	td.className = 'empty';

	return td;
}

// Change add row/delete button state
function update_buttons(state) {
	$('#add').prop('disabled', !state);
	$('#delete').prop('disabled', !state);
	$('#delete_id').prop('disabled', !state);
}

// Transform data set by modifying foreign key values to "Name (ID)"
function transform_data(data, columns) {

	// build autocomplete data and change data array
	for(var i in column_names) {
		var column = column_names[i];

		// Check for foreign key reference
		if(references.hasOwnProperty(column)) {
			var reference = references[column];
			var names = []

			// Build drop down
			for(var key in reference) {
				if(reference.hasOwnProperty(key)) {
					var value = reference[key] + " (" + key + ")";
					names.push(value);
				}
			}

			// Add to columns list
			columns.push({
				type: 'dropdown',
				source: names,
			});

			// Change data
			for(var data_row in data) {
				var id = data[data_row][i];
				data[data_row][i] = reference[id] + " (" + id + ")";
			}
		}
		else
			columns.push({});
	}

	// Add children links
	if(Object.keys(children).length > 0) {
		column_names.push('');
		columns.push({ renderer: link_renderer });
		for(var row in data) {
			var id = data[row][0];
			var link = '';
			for(var child in children) {
				var field_name = children[child][0];
				link += '<a href="/?table=' + child + '&' + field_name + '=' + id + '">' + child + '</a>&nbsp;';
			}

			data[row].push(link);
		}
	}
}

// Reload data
function reload() {
	$.getJSON(data_url, function(response) {
		var data = response['data'];
		var columns = [];
		transform_data(data, columns)

		hot.loadData(data);
		update_buttons(1);
	});
}

// Save data
function save() {

	// Convert dropdowns back to ids
	var headers = hot.getColHeader();
	var data = hot.getData();
	for(var row in data) {
		for(var col in headers) {

			// Remove empty columns from data array
			if(column_names[col] == "") {
				data[row].splice(col, 1);
				continue;
			}

			// Set null values to empty string
			if(data[row][col] == null)
				data[row][col] = "";

			// Parse id from dropdown data
			if(references.hasOwnProperty(headers[col])) {
				match = data[row][col].match(/\((.*?)\)$/);
				if(match)
					data[row][col] = parseInt(match[1]);
			}
		}
	}

	// Send request
	var request = $.ajax({
		type: "POST",
		url: "/save" + querystring,
		data: jQuery.param({'data': data}),
		dataType: "html",
		success: function(response, text_status, jqxhr) {
			ajax_success(response, text_status, jqxhr);
			update_buttons(1);
		},
		error: ajax_error,
	});
}

// Add new row
function add() {

	// Send request
	var request = $.ajax({
		type: "POST",
		url: "/add" + querystring,
		data: {},
		dataType: "html",
		success: function(response, text_status, jqxhr) {
			ajax_success(response, text_status, jqxhr);
			reload();
		},
		error: ajax_error,
	});
}

// Delete row
function remove() {
	id = parseInt($('#delete_id').val());
	if(id <= 0 || isNaN(id) || !confirm("Sure ?"))
		return;

	// Send request
	var request = $.ajax({
		type: "POST",
		url: "/remove" + querystring,
		data: {'id': id},
		dataType: "html",
		success: function(response, text_status, jqxhr) {
			ajax_success(response, text_status, jqxhr);
			reload();
		},
		error: ajax_error,
	});
}

function ajax_success(response, text_status, jqxhr) {
	result = JSON.parse(response);
	if(text_status == "success") {
		message = result['message'];
		if(result['message'] != undefined)
			$('#message').html(message);
	}
	else
		$('#message').html(text_status);
}

function ajax_error(jqxhr, text_status, error) {
	$('#message').html(error);
}

// Ctrl+s hotkey
$(document).keydown(function(event) {
	if(!(String.fromCharCode(event.which).toLowerCase() == 's' && event.ctrlKey) && !(event.which == 19))
		return true;

	event.preventDefault();
	save();

	return false;
});

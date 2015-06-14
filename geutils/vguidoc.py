from sys import stdout

factories = {
	"AnalogBar" 			: "Panel",
	"AnimatingImagePanel" 	: "Panel",
	"Button" 				: "Label",
	"CAvatarImagePanel" 	: "ImagePanel",
	"CBitmapImagePanel" 	: "Panel",
	"CCommentaryModelPanel" : "EditablePanel",
	"CControllerMap" 		: "Panel",
	"CIconPanel" 			: "Panel",
	"CModelPanel" 			: "EditablePanel",
	"CTreeViewListControl"	: "Panel",
	"CheckButton" 			: "ToggleButton",
	"CircularProgressBar" 	: "ProgressBar",
	"ComboBox" 				: "TextEntry",
	"ContinuousAnalogBar" 	: "AnalogBar",
	"ContinuousProgressBar" : "ProgressBar",
	"Divider" 				: "Panel",
	"EditablePanel" 		: "Panel",
	"ExpandButton" 			: "ToggleButton",
	"Frame"					: "Panel",
	"GECreditsText" 		: "RichText",
	"GEHorzListPanel" 		: "Panel",
	"GEVertListPanel" 		: "Panel",
	"GraphPanel" 			: "Panel",
	"ImageButton" 			: "Button",
	"ImagePanel" 			: "Panel",
	"Label" 				: "Panel",
	"ListPanel" 			: "Panel",
	"ListViewPanel" 		: "Panel",
	"Menu" 					: "Panel",
	"MenuBar" 				: "Panel",
	"MenuButton" 			: "Button",
	"MenuItem" 				: "Button",
	"MessageBox" 			: "Frame",
	"Panel" 				: None,
	"PanelListPanel" 		: "Panel",
	"ProgressBar" 			: "Panel",
	"RadioButton" 			: "ToggleButton",
	"RichText" 				: "Panel",
	"RotatingProgressBar" 	: "ProgressBar",
	"ScalableImagePanel" 	: "Panel",
	"ScrollBar" 			: "Panel",
	"SectionedListPanel" 	: "Panel",
	"TextEntry" 			: "Panel",
	"ToggleButton" 			: "Button",
	"TreeView" 				: "Panel",
	"URLLabel" 				: "Label",
}

# Variables are stored per panel that define them
# Each variable is stored as a tuple in the following format:
#	( name, default, type, notes )
variables = {
	"Panel" : [
		("fieldName", "", "string", ""),
		("xpos", "0", "int", "Prepend with r for right align, c for center align"),
		("ypos", "0", "int", "Prepend with r for right align, c for center align"),
		("zpos", "0", "int", ""),
		("wide", "64", "int", "Prepend with f to set wide = screenWidth - wide"),
		("tall", "24", "int", ""),
		("visible", "1", "0|1", ""),
		("enabled", "1", "0|1", ""),
		("alpha", "255", "0-255", ""),
		("paintborder", "-1", "0|1", ""),
		("paintbackground", "-1", "0|1", ""),
		("PaintBackgroundType", "0", "0|1|2", "0 = color, 1 = texture, 2 = rounded  corner"),
		("Texture1", "vgui/hud/800corner1", "string", ""),
		("Texture2", "vgui/hud/800corner1", "string", ""),
		("Texture3", "vgui/hud/800corner1", "string", ""),
		("Texture4", "vgui/hud/800corner1", "string", ""),
		("IgnoreScheme", "0", "0|1", "Don't load scheme colors"),
		("tabPosition", "0", "int", ""),
		("tooltiptext", "", "string", ""),
	],
	"Frame" : [
		("settitlebarvisible", "1", "0|1", ""),
		("title", "", "string", ""),
		("title_font", "", "font", ""),
		("titletextinsetX", "0", "int", ""),
		("titletextinsetY", "0", "int", ""),
		("clientinsetx_override", "-1", "int", ""),
	],
	"AnimatingImagePanel" : [
		("image", "", "string", ""),
		("scaleImage", "0", "0|1", ""),
		("frames", "0", "int", "Number of frames"),
		("anim_framerate", "100", "int", "Milliseconds per frame"),
	],
	"ImagePanel" : [
		("image", "", "string", ""),
		("scaleImage", "0", "0|1", ""),
		("scaleAmount", "0.0", "float", ""),
		("tileImage", "0", "0|1", ""),
		("tileHorizontally", "tileImage", "0|1", ""),
		("tileVertically", "tileImage", "0|1", ""),
		("fillcolor", "", "string|color", ""),
		("border", "", "string", ""),
	],
	"ScalableImagePanel" : [
		("src_corner_height", "0", "int", ""),
		("src_corner_width", "0", "int", ""),
		("draw_corner_height", "0", "int", ""),
		("draw_corner_width", "0", "int", ""),
		("image", "", "string", ""),
	],
	"Label" : [
		("labelText", "", "string", "%string% creates localization #var_[string] = string"),
		("font", "Default", "string", ""),
		("textAlignment", "", "north-west|north|north-east|west|center|east|south-west|south|south-east", ""),
		("dulltext", "0", "0|1", ""),
		("brighttext", "0", "0|1", ""),
		("wrap", "0", "0|1", ""),
		("centerwrap", "0", "0|1", ""),
		("textinsetx", "0", "int", ""),
		("textinsety", "0", "int", ""),
		("associate", "", "string", ""),
	],
	"Button" : [
		("command", "", "string", ""),
		("default", "0", "0|1", ""),
		("selected", "-1", "0|1", ""),
		("sound_armed", "", "string", ""),
		("sound_depressed", "", "string", ""),
		("sound_released", "", "string", ""),
	],
	"ImageButton" : [
		("defaultimg", "", "string", ""),
		("armedimg", "", "string", ""),
		("depressedimg", "", "string", ""),
	],
	"RadioButton" : [
		("SubTabPosition", "0", "int", ""),
		("TabPosition", "0", "int", ""),
	],
	"CModelPanel" : [
		("fov", "54", "int", ""),
		("start_framed", "0", "0|1", ""),
		("allow_offscreen", "0", "0|1", ""),
		("model", "", "KeyValues", "Starts a section: model { }"),
		("modelname", "", "string", "In model"),
		("modelname_hwm", "", "string", "In model"),
		("skin", "-1", "int", "In model"),
		("angles_x", "0.0", "float", "In model"),
		("angles_y", "0.0", "float", "In model"),
		("angles_z", "0.0", "float", "In model"),
		("origin_x", "110.0", "float", "In model"),
		("origin_y", "5.0", "float", "In model"),
		("origin_z", "5.0", "float", "In model"),
		("frame_origin_x", "110.0", "float", "In model"),
		("frame_origin_y", "5.0", "float", "In model"),
		("frame_origin_z", "5.0", "float", "In model"),
		("vcd", "", "string", "In model"),
		("spotlight", "0", "int", "In model"),
		("animation", "", "KeyValues", "In model; Starts a section: animation { }"),
		("name", "", "string", "In animation"),
		("sequence", "0", "int", "In animation"),
		("activity", "0", "int", "In animation"),
		("default", "0", "0|1", "In animation"),
		("pose_parameters", "", "KeyValues", "In animation; Enter KeyValues pose parameters"),
		("attached_model", "", "KeyValues", "In model; Starts a section: attached_model { }"),
		("modelname", "", "string", "In attached_model"),
		("skin", "-1", "int", "In acttached_model"),
	],
	"CIconPanel" : [
		("icon", "", "hud icon", ""),
		("scaleImage", "0", "0|1", ""),
	],
	"CBitmapImagePanel" : [
		("image", "", "string", ""),
		("imagecolor", "", "color", ""),
		("imageAlignment", "", "north-west|north|north-east|west|center|east|south-west|south|south-east", ""),
		("preserveAspectRatio", "0", "0|1", ""),
		("filtered", "0", "0|1", "Hardware filtered"),
	],
	"ProgressBar" : [
		("progress", "0.0", "float", ""),
		("variable", "", "string", "A certain dialog variable"),
	],
	"AnalogBar" : [
		("analogValue", "0.0", "float", ""),
		("variable", "", "string", "A certain dialog variable"),
	],
	"CircularProgressBar" : [
		("fg_image", "", "string", ""),
		("bg_image", "", "string", ""),
	],
	"RotatingProgressBar" : [
		("image", "", "string", ""),
		("start_degrees", "0.0", "float", ""),
		("end_degrees", "0.0", "float", ""),
		("approach_speed", "360.0", "float", ""),
		("rot_origin_x_percent", "0.5", "float", ""),
		("rot_origin_y_percent", "0.5", "float", ""),
		("rotating_x", "0", "int", ""),
		("rotating_y", "0", "int", ""),
		("rotating_wide", "0", "int", ""),
		("rotating_tall", "0", "int", ""),
	],
	"SectionedListPanel" : [
		("linespacing", "0", "int", ""),
	],
	"TextEntry" : [
		("textHidden", "0", "0|1", ""),
		("editable", "1", "0|1", ""),
		("maxchars", "-1", "int", ""),
		("NumericInputOnly", "0", "0|1", ""),
		("unicode", "0", "0|1", ""),
	],
	"URLLabel" : [
		("URLText", "", "string", ""),
	],
	"RichText" : [
		("maxchars", "-1", "int", ""),
		("scrollbar", "1", "0|1", ""),
		("text", "", "string", ""),
		("textfile", "", "string", ""),
	],
	"GECreditsText" : [
		("font", "Default", "string", ""),
	],
}


def ListFactories():
	ClearFile()
	
	factories_sort = sorted(factories.keys())
	factory_list = ""
	for f in factories_sort:
		factory_list += "<a href=\"#%s\">%s</a>, " %(f, f)
	
	WriteToFile( factory_list + "<br/>" )
	
	for f in factories_sort:
		ShowVariables( f )

def ShowVariables( factory ):
	if factory not in factories:
		print "Factory %s doesn't exist" % factory
		return
		
	WriteToFile( "<br/><hr/>" )
		
	ShowHierarchy( factory )
	
	WriteToFile( "<table border=1 cellpadding=2><tr><th>Variable</th><th>Default</th><th>Type</th><th>Notes</th></tr>" )
	
	if factory in variables:
		PrintVariables( factory )
		
	child = factories[factory]
	while child is not None:
		PrintVariables( child )
		child = factories[child]
		
	WriteToFile( "</table>" )
	
	print "Variables outputted successfully [%s]" % factory
		
def ShowHierarchy( factory ):
	if not factory in factories:
		print "Factory %s doesn't exist" % factory
		return
		
	output = "<br/><a name=\"%s\"></a>" % factory
	
	# Output the factory
	output += "<a href=\"#%s\">%s</a>" % (factory, factory)
	
	# Output our children
	child = factories[factory]
	while child is not None:
		output += " -> <a href=\"#%s\">%s</a>" % (child, child)
		child = factories[child]
	
	output += "<br/><br/>"
	
	WriteToFile( output )

def PrintVariables( factory ):
	if factory in variables:
		for var in variables[factory]:
			WriteToFile( "<tr><td>%s</td><td>(%s)</td><td>[%s]</td><td>%s</td>" % var )
				
def ClearFile():
	file = open( 'vguidoc.html', 'w' )
	file.close()
	
def WriteToFile( text ):
	file = open( 'vguidoc.html', 'a' )
	file.write( text + "\n" )
	file.close()
	
// SWF version, DO NOT TOUCH, used to make sure the SWF matches the dll.
var VERSION:Number = 6;
//Globals
var maxRows:Number = 4;
var spacingFromSelected = 5;
var spacingBetweenEquippedX = 5;
var spacingBetweenEquippedY = 5;
var counter:Number = 0;
var selectedCounter:Number = 0;
var keycode:Number;
var lastBoolean:Boolean;
var displayButton:Boolean;
var openedMenuName:String;
var actor:String = "";
var ALPHA:Number = 100;
var diffOffset:Number = 0;
var valueOffset:Number = 2;
var columnTwoOffset:Number = 155;
var DEFAULT_ITEM_NAME_X:Number;
var layout = 0;
var SCROLL_SPEED:Number = 100;
var SCROLL_DELAY:Number = 2.5;
var effectsOriginalHeight = 54;
var effectsOriginalWidth = 260;
//Gloabls: Strings
var buttonCompareText = "Compare";
var qlieHintText = "Compare";
var comparingTo = "Comparing to";
var damageLabelText = "Damage";
var critLabelText = "Crit";
var noneText = "None";
var armorTypeLabelText = "Armor Type";
var armorRatingLabelText = "Armor Rating";
var goldLabelText = "Gold Value";
var effectsLabelText = "Effect(s)";
var equippedTo = "Equipped to";
var slotsLabelText = "Slot(s)";
var speedLabelText = "Speed";
var reachLabelText = "Reach";
var staggerLabelText = "Stagger";
var keyText = "Key";
var keyInfo = "(Total: T,  Average: A,  Max: M)";
var totalKey = "T";
var averageKey = "A";
var maxKey = "M";
var notApplicable = "N/A"
var betterWrapperStart = "<font color=\"#00FF00\">(+";
var betterWrapperEnd = ")</font>";
var worseWrapperStart = "<font color=\"#FF0000\">(";
var worseWrapperEnd = ")</font>";
var normalWrapperStart = "<font color=\"#FFFFFF\">";
var normalWrapperEnd = "</font>";

var warningLabel = "VERSION MISMATCH";
var dllExpectsText = "CompareEquipmentNG.dll expects SWF version";
var swfVersionText = "This SWF is version";
var swfOutdatedWarning = "This SWF is outdated and needs an update.\n"+
						 "Unintended behavior may occur if this warning\n"+
						 "is ignored. Check your translation/re-skin mod.";
var dllOutdatedWarning = "This SWF is newer than your CompareEquipmentNG.dll\n"+
						 "and you may need to update Compare Equipment NG.";

//saved menu_mc objects;
//var menu:MovieClip;
var inventoryLists:MovieClip;
var navPanel:MovieClip;
var skyUiItemCard:MovieClip;
var AHZItemCardContainer:MovieClip;
var weaponStatsV2:MovieClip;
//saved quickloot objects;
var inLootMenu:Boolean = false;
var lootMenuButtonBar:MovieClip;
var lootMenuDataProvider:Array;


function setVariables(){
	var i = 0;
	buttonCompareText = arguments[i++];
	comparingTo = arguments[i++];
	damageLabelText = arguments[i++];
	critLabelText = arguments[i++];
	noneText = arguments[i++];
	armorTypeLabelText = arguments[i++];
	armorRatingLabelText = arguments[i++];
	goldLabelText = arguments[i++];
	effectsLabelText = arguments[i++];
	equippedTo = arguments[i++];
	slotsLabelText = arguments[i++];
	speedLabelText = arguments[i++];
	reachLabelText = arguments[i++];
	staggerLabelText = arguments[i++];
	keyText = arguments[i++];
	keyInfo = arguments[i++];
	totalKey = arguments[i++];
	averageKey = arguments[i++];
	maxKey = arguments[i++];
	notApplicable = arguments[i++];
	betterWrapperStart = arguments[i++];
	betterWrapperEnd = arguments[i++];
	worseWrapperStart = arguments[i++];
	worseWrapperEnd = arguments[i++];
	normalWrapperStart = arguments[i++];
	normalWrapperEnd = arguments[i++];
	diffOffset = arguments[i++];
	valueOffset = arguments[i++];
	columnTwoOffset = arguments[i++];
	spacingFromSelected = arguments[i++];
	spacingBetweenEquippedX = arguments[i++]
	spacingBetweenEquippedY = arguments[i++];
	layout = arguments[i++];
}

function normalWrap(str:String){
	return normalWrapperStart + str + normalWrapperEnd;
}

function betterWrap(str:String){
	return betterWrapperStart + str + betterWrapperEnd;
}

function worseWrap(str:String){
	return worseWrapperStart + str + worseWrapperEnd;
}

function setWeaponLabels(instanceToLabel:MovieClip){
	var tb = instanceToLabel.TextBoxes;
	tb.ActorLabel.Text.htmlText = comparingTo;
	tb.SlotLabel.Text.htmlText = slotsLabelText;
	tb.TypeLabel.Text.htmlText = damageLabelText;
	tb.RatingLabel.Text.htmlText = critLabelText;
	tb.GoldLabel.Text.htmlText = goldLabelText;
	tb.SpeedLabel.Text.htmlText = speedLabelText;
	tb.StaggerLabel.Text.htmlText = staggerLabelText;
	tb.ReachLabel.Text.htmlText = reachLabelText;
	tb.EffectsLabel.Text.htmlText = effectsLabelText;
}

function setArmorLabels(instanceToLabel:MovieClip){
	var tb = instanceToLabel.TextBoxes;
	tb.ActorLabel.Text.htmlText = comparingTo;
	tb.SlotLabel.Text.htmlText = slotsLabelText;
	tb.TypeLabel.Text.htmlText = armorTypeLabelText;
	tb.RatingLabel.Text.htmlText = armorRatingLabelText;
	tb.GoldLabel.Text.htmlText = goldLabelText;
	tb.EffectsLabel.Text.htmlText = effectsLabelText;

}

var DEFAULT_ICON_SOURCE:String = "skyui/icons_item_psychosteve.swf";
var DEFAULT_ICON_LABEL:String = "default_misc";
var DEFAULT_ICON_COLOR:Number = 0xFFFFFF;

var ICON_SIZE:Number = 20;
var _iconLoader: MovieClipLoader;
var _iconColor: Number = DEFAULT_ICON_COLOR;

function loadIcon(instanceI: MovieClip, iconLabel:String, iconSource:String, iconColor:Number){
	_iconLoader = new MovieClipLoader();
	var listenerObject = new Object();

	if (iconSource == undefined)
		iconSource = DEFAULT_ICON_SOURCE;

	if (iconColor == undefined)
		iconColor = DEFAULT_ICON_COLOR;

	if (iconLabel == DEFAULT_ICON_LABEL)
	{
		instanceI.ItemIcon._visible = false;
		instanceI.TextBoxes.ItemName._x = 0;
		return;
	}
	instanceI.TextBoxes.ItemName._x = DEFAULT_ITEM_NAME_X;
	
	listenerObject.onLoadInit = function(icon: MovieClip)
	{
		icon.gotoAndStop(iconLabel);
		
		icon._visible = true;
		
		var colorTransform = new flash.geom.ColorTransform();
		if(iconColor instanceof Number) {
			colorTransform.rgb = iconColor;
		}
		else if (iconColor != undefined) {
			colorTransform.rgb = parseInt(iconColor.toString());
		}
		
		
		var transform = new flash.geom.Transform(MovieClip(icon));
		transform.colorTransform = colorTransform;
		_root.ce_assets.ICON_SIZE = _root.ce_assets.SelectedItemCard.TextBoxes.ItemName._height;
		icon._width = _root.ce_assets.ICON_SIZE;
		icon._height = _root.ce_assets.ICON_SIZE;
	};
	_iconLoader.addListener(listenerObject);
	_iconLoader.loadClip(iconSource, instanceI.ItemIcon);
}

// For adding button hint
function addButton() {
	if (navPanel != null){
		navPanel.addButton({text: buttonCompareText, controls: { keyCode: keycode } });
		navPanel.updateButtons(true);
	}
}

//Converts a number into a string or float with a precision of two
function precisionTwo(num:Number){
	var split:Array= num.toString().split('.');
	if (split.length == 1)
	{
		return num + '.00';
	}
	var end = split[1].substr(0, 2);
	return parseFloat(split[0] + '.' + end);
}

function onUnload(){
	//So that the button doesn't duplicate
	if (_parent != null){
		_parent.updateBottomBar = _parent.ce_original_UpdateBottomBar;
		_parent.ce_original_UpdateBottomBar = undefined;
	}
	//So the new function doesn't duplicate
	if (lootMenuButtonBar != null){
		lootMenuButtonBar.invalidateData = lootMenuButtonBar.ce_original_invalidateData;
		lootMenuButtonBar.ce_original_invalidateData = undefined;
	}
	//So the new function doesn't duplicate
	if (weaponStatsV2 != null){
		weaponStatsV2.showTheStats = weaponStatsV2.ce_original_showTheStats
		weaponStatsV2.ce_original_showTheStats = undefined;
	}
	if (_root.ce_assets != null){
		_root.ce_assets.removeMovieClip();
	}
}

function onLoad() {
	var loadListener:Object = new Object();
	var loader:MovieClipLoader = new MovieClipLoader();
	loader.addListener(loadListener);
	
	loadListener.onLoadInit = function(mc:MovieClip) {
		mc.attachMovie("ItemCard", "SelectedItemCard", mc.getNextHighestDepth());
		mc.SelectedItemCard._visible = false;
		loaded();
	};
	loader.loadClip("CompareEquipment_assets.swf", _root.createEmptyMovieClip("ce_assets", _root.getNextHighestDepth()));
}

function loaded(){
	inventoryLists = _parent.inventoryLists;
	DEFAULT_ITEM_NAME_X = _root.ce_assets.SelectedItemCard.TextBoxes.ItemName._x;
	navPanel = _parent.navPanel;
	skyUiItemCard = _parent.itemCard;
	AHZItemCardContainer = _root.AHZItemCardContainer;
	var args:Array = this._name.split("_");
    var expectedSwfVersion:Number = _parent.ce_expectedVersion;
    if (args.length > 2){
        expectedSwfVersion = 5;
    }
	openedMenuName = args[1];
	
	getVariables();
	
	lootMenuButtonBar = _root.lootMenu.buttonBar;
	if (lootMenuButtonBar != undefined && lootMenuButtonBar.ce_original_invalidateData == undefined){
		inLootMenu = true;
		lootMenuDataProvider = lootMenuButtonBar.dataProvider;
		lootMenuButtonBar.ce_original_invalidateData = lootMenuButtonBar.invalidateData;
		lootMenuButtonBar.invalidateData = function(){
				if(cengIsWeaponOrArmor()){
					var object:Object = lootMenuDataProvider[lootMenuDataProvider.length - 1];
					if (object != undefined && object.ce_button == undefined){
						var buttonObject:Object = new Object();
						buttonObject.label = qlieHintText;
						buttonObject.index = keycode;
						buttonObject.stolen = false;
						buttonObject.ce_button = true;
						lootMenuButtonBar.ce_buttonObject = buttonObject;
						lootMenuDataProvider.push(lootMenuButtonBar.ce_buttonObject);
					}
				}
				else{
					var object:Object = lootMenuDataProvider[lootMenuDataProvider.length - 1];
					if (object.ce_button != undefined){
						lootMenuDataProvider.pop();
						lootMenuButtonBar.ce_button = undefined;
					}
				}
				lootMenuButtonBar.ce_original_invalidateData();
			}
		showQLIEhint();
	}
	
	//---------------For adding button hint, thanks shazdeh2 ------
	var _thisMenuName = this._name;
	_parent.ce_original_UpdateBottomBar = _parent.updateBottomBar;
	_parent.updateBottomBar = function(arg:Boolean) {
			this.ce_original_UpdateBottomBar(arg);
			this[_thisMenuName].lastBoolean = arg;
			if (this[_thisMenuName].displayButton){
				this[_thisMenuName].addButton();
			}
		}
	//-------------------------------------------------------------
	if (expectedSwfVersion == VERSION){
		_root.ce_assets._visible = false;
		effectsOriginalHeight = _root.ce_assets.SelectedItemCard.TextBoxes.Effects._height - 2;
		effectsOriginalWidth = _root.ce_assets.SelectedItemCard.TextBoxes.Effects._width;
		addListeners();
		reset();
		//-- Prevents button hit diseapering on setting reload ---
		if (_parent != null && _parent != undefined){
			if (_parent.ce_lastBoolean != undefined){
				lastBoolean = _parent.ce_lastBoolean;
			}
			if (_parent.ce_lastEvent != undefined){
				showOrHideHotkeyHint(_parent.ce_lastEvent);
			}
		}
        if(openedMenuName == "HUDMenu"){
		    cengGetArmorOrWeapon();
	    }
		//--------------------------------------------------------
	}
	else{
		//----------------Version mismatch warning----------------
		reset();
		_root.ce_assets._visible = true;
        _root.ce_assets.SelectedItemCard._visible = true;
		var tb = _root.ce_assets.SelectedItemCard.TextBoxes;
		tb.ActorLabel.Text.text = "";
		tb.ActorName.Text.text = warningLabel;
		tb.ActorName.Text.textColor = 0xFF0000;
		tb.ItemName.Text.text = dllExpectsText + ": " + expectedSwfVersion;
		tb.ItemName.Text.textColor = 0xFF0000;
		tb.SlotNumbers.Text.text = swfVersionText + ": " + VERSION;
		tb.SlotNumbers.Text.textColor = 0xFF0000;
		if (expectedSwfVersion > VERSION){
			tb.Effects.Text.text = swfOutdatedWarning;
		}
		else{
			tb.Effects.Text.text = dllOutdatedWarning;
		}

		tb.Effects.Text.textColor = 0xFF0000;
		//----------------------------------------------------------
	}
}

function getVariables(){
    keycode = _parent.ce_hotKey;
	buttonCompareText = _parent.ce_buttonCompareText;
	qlieHintText = _parent.ce_qlieHintText;
	comparingTo = _parent.ce_comparingTo;
	damageLabelText = _parent.ce_damageLabelText;
	critLabelText = _parent.ce_critLabelText;
	noneText = _parent.ce_noneText;
	armorTypeLabelText = _parent.ce_armorTypeLabelText;
	armorRatingLabelText = _parent.ce_armorRatingLabelText;
	goldLabelText = _parent.ce_goldLabelText;
	effectsLabelText = _parent.ce_effectsLabelText;
	equippedTo = _parent.ce_equippedTo;
	slotsLabelText = _parent.ce_slotsLabelText;
	speedLabelText = _parent.ce_speedLabelText;
	reachLabelText = _parent.ce_reachLabelText;
	staggerLabelText = _parent.ce_staggerLabelText;
	keyText = _parent.ce_keyText;
	keyInfo = _parent.ce_keyInfo;
	totalKey = _parent.ce_totalKey;
	averageKey = _parent.ce_averageKey;
	maxKey = _parent.ce_maxKey;
	notApplicable = _parent.ce_notApplicable;
	betterWrapperStart = _parent.ce_betterWrapperStart;
	betterWrapperEnd = _parent.ce_betterWrapperEnd;
	worseWrapperStart = _parent.ce_worseWrapperStart;
	worseWrapperEnd = _parent.ce_worseWrapperEnd;
	normalWrapperStart = _parent.ce_normalWrapperStart;
	normalWrapperEnd = _parent.ce_normalWrapperEnd;
	diffOffset = _parent.ce_diffOffset;
	valueOffset = _parent.ce_valueOffset;
	columnTwoOffset = _parent.ce_columnTwoOffset;
	spacingFromSelected = _parent.ce_SPACING_FROM_SELECTED;
	spacingBetweenEquippedX = _parent.ce_SPACING_BETWEEN_EQUIPPED_X;
	spacingBetweenEquippedY = _parent.ce_SPACING_BETWEEN_EQUIPPED_Y;
	layout = _parent.ce_layout;
	SCROLL_SPEED = _parent.ce_SCROLL_SPEED;
	SCROLL_DELAY = _parent.ce_SCROLL_DELAY;
	ALPHA = _parent.ce_alpha;
	_root.ce_assets._xscale = _parent.ce_scale;
	_root.ce_assets._yscale = _parent.ce_scale;
	maxRows = _parent.ce_maxRows;
	_root.ce_assets.SelectedItemCard.Background._alpha = ALPHA;
}

function showOrHideQLIEHint(){
	if(inLootMenu == false)
		return;
	lootMenuButtonBar.invalidateData();
}

function hideSkyUiItemCardAndMore(){
	if(skyUiItemCard != undefined){
		skyUiItemCard._visible = false;
		skyUiItemCard.ce_hidden = true;
	}
	if (weaponStatsV2 != undefined){
		weaponStatsV2._visible = false;
		weaponStatsV2.ce_hidden = true;
	}
	if (AHZItemCardContainer != undefined){
		AHZItemCardContainer._visible = false;
		AHZItemCardContainer.ce_hidden = true;
	}
}

function addListeners() {
	inventoryLists.addEventListener("categoryChange", this, "showOrHideHotkeyHint");
	inventoryLists.addEventListener("itemHighlightChange", this, "hideAndReset");
}

function cengPersistentDisplay(){
	skse.plugins.CompareEquipment.persistentDisplayAS("");
}

function cengIsWeaponOrArmor(){
	return skse.plugins.CompareEquipment.isWeaponOrArmorAS("");
}

function cengGetArmorOrWeapon(){
	skse.plugins.CompareEquipment.getArmorOrWeaponAS("");
}

function loggerAS(str:String){
	skse.plugins.CompareEquipment.loggerAS(str);
}

function showOrHideHotkeyHint(event: Object){
	if (_parent != null){
		_parent.ce_lastEvent = event;
	}
	_root.ce_assets._visible = false;
	reset();
	if (navPanel != null){
		navPanel.clearButtons();
	}

	displayButton = false;
	
	//Only display button hint in valid menus All = 0|11, Weapons = 1|12, Armor == 2|13
	if (openedMenuName == "InventoryMenu" && 
		(event.index == 0 || event.index == 1 || event.index == 2 || event.index == 3)){
		displayButton = true;
	}
	if ((openedMenuName == "ContainerMenu" || openedMenuName == "BarterMenu") && 
		(event.index == 0 || event.index == 1 || event.index == 2 ||
		 event.index == 11 || event.index == 12 || event.index == 13)){
		displayButton = true;
	}
	if (openedMenuName == "GiftMenu"){
		displayButton = true;
	}
	//----------------------------------------------------------------------------------
	
	//Display button hint with same as last called boolean
	if (_parent != null){
		_parent.updateBottomBar(lastBoolean);
	}
}

function hideAndReset(){
	//Save last boolean to menu_mc for when a settings reload occurs.
	if (_parent != null){
		_parent.ce_lastBoolean = lastBoolean;
	}
	_root.ce_assets._visible = false;
	if (skyUiItemCard != null && displayButton && skyUiItemCard.ce_hidden == true){
		skyUiItemCard._visible = true;
		skyUiItemCard.ce_hidden = false;
	}
	if (AHZItemCardContainer != null && AHZItemCardContainer.ce_hidden == true){
		AHZItemCardContainer._visible = true;
		AHZItemCardContainer.ce_hidden = false;
	}
	if (weaponStatsV2 == undefined){
		weaponStatsV2 = _parent.WeaponStats.WeaponStats_MC;
		if (weaponStatsV2 != undefined){
			weaponStatsV2.ce_original_showTheStats = weaponStatsV2.showTheStats;
			weaponStatsV2.showTheStats = function(){
				if (weaponStatsV2.ce_hidden != true){
					weaponStatsV2.ce_original_showTheStats();
				}
			}
		}
	}
	if (weaponStatsV2 != null && weaponStatsV2.ce_hidden == true){
		weaponStatsV2.ce_hidden = false;
	}
	
	reset();
	cengPersistentDisplay();
}

function effectsScroller(tb){
	tb.Effects.Text.autoSize= true;
	tb.Effects.Text.multiline = true;
	tb.Effects.Text.wordWrap = true;
	var emc = tb["effectsMaskClip"];
	emc.removeMovieClip();
	var effectsMaskClip:MovieClip = tb.createEmptyMovieClip("effectsMaskClip", tb.getNextHighestDepth());
	effectsMaskClip.beginFill(0xFF0000);
	effectsMaskClip.moveTo(0, 0);
	effectsMaskClip.lineTo(effectsOriginalWidth, 0);
	effectsMaskClip.lineTo(effectsOriginalWidth, effectsOriginalHeight);
	effectsMaskClip.lineTo(0, effectsOriginalHeight);
	effectsMaskClip.lineTo(0, 0);
	effectsMaskClip.endFill();
	effectsMaskClip._visible = true;
	effectsMaskClip._x = tb.Effects._x;
	effectsMaskClip._y = tb.Effects._y;
	tb.Effects.setMask(effectsMaskClip);
	var startDelay:Number = -1;
	var endDelay:Number = -1;
	var startTime:Number = -1;
	function scrollText(){
		if (tb.Effects._height <= effectsOriginalHeight){
			return;
		}
		var currentTime:Number = getTimer();
		if (startTime == -1){
			startTime = currentTime;
		}
		if (startDelay < SCROLL_DELAY){
			startDelay = (currentTime - startTime) / 1000;
		}
		else if(startDelay >= SCROLL_DELAY && startDelay != 123456789){
			startDelay = 123456789;
			startTime = currentTime;
		}
		else if (-tb.Effects.Text._y + effectsOriginalHeight < tb.Effects.Text._height){
			tb.Effects.Text._y = -((currentTime - startTime) / SCROLL_SPEED);
		}
		else if(startDelay >= SCROLL_DELAY && endDelay == -1){
			startTime = currentTime;
			endDelay = (currentTime - startTime) / 1000;
		}
		else if(endDelay < SCROLL_DELAY){
			endDelay = (currentTime - startTime) / 1000;
		}
		else{
			tb.Effects.Text._y = 0;
			startTime = currentTime;
			endDelay = -1;
			startDelay = -1;
		}
	}
	tb.Effects.onEnterFrame = scrollText;
	tb.Effects.Text._y = 0;
}

function itemNameScroller(tb){
	tb.ItemName.Text.autoSize= true;
	tb.ItemName.Text.multiline = false;
	tb.ItemName.Text.wordWrap = false;
	var inmc = tb["itemNameMaskClip"];
	inmc.removeMovieClip();
	var itemNameMaskClip:MovieClip = tb.createEmptyMovieClip("itemNameMaskClip", tb.getNextHighestDepth());
	var itemNameMaxWidth = 270 - tb.ItemName._x;
	itemNameMaskClip.beginFill(0xFF0000);
	itemNameMaskClip.moveTo(0, 0);
	itemNameMaskClip.lineTo(itemNameMaxWidth, 0);
	itemNameMaskClip.lineTo(itemNameMaxWidth, tb.ItemName._height);
	itemNameMaskClip.lineTo(0, tb.ItemName._height);
	itemNameMaskClip.lineTo(0, 0);
	itemNameMaskClip.endFill();
	itemNameMaskClip._visible = true;
	itemNameMaskClip._x = tb.ItemName._x;
	itemNameMaskClip._y = tb.ItemName._y;
	tb.ItemName.setMask(itemNameMaskClip);
	var startDelay:Number = -1;
	var endDelay:Number = -1;
	var startTime:Number = -1;
	function scrollText(){
		if (tb.ItemName._width <= itemNameMaxWidth){
			return;
		}
		var currentTime:Number = getTimer();
		if (startTime == -1){
			startTime = currentTime;
		}
		if (startDelay < SCROLL_DELAY){
			startDelay = (currentTime - startTime) / 1000;
		}
		else if(startDelay >= SCROLL_DELAY && startDelay != 123456789){
			startDelay = 123456789;
			startTime = currentTime;
		}
		else if (-tb.ItemName.Text._x + itemNameMaxWidth < tb.ItemName.Text._width){
			tb.ItemName.Text._x = -((currentTime - startTime) / (SCROLL_SPEED * 0.5));
		}
		else if(startDelay >= SCROLL_DELAY && endDelay == -1){
			startTime = currentTime;
			endDelay = (currentTime - startTime) / 1000;
		}
		else if(endDelay < SCROLL_DELAY){
			endDelay = (currentTime - startTime) / 1000;
		}
		else{
			tb.ItemName.Text._x = 0;
			startTime = currentTime;
			endDelay = -1;
			startDelay = -1;
		}
	}
	tb.ItemName.onEnterFrame = scrollText;
	tb.ItemName.Text._x = 0;
}

function actorNameScroller(tb){
	tb.ActorName.Text.autoSize= true;
	tb.ActorName.Text.multiline = false;
	tb.ActorName.Text.wordWrap = false;
	var anmc = tb["actorNameMaskClip"];
	anmc.removeMovieClip();
	var actorNameMaskClip:MovieClip = tb.createEmptyMovieClip("actorNameMaskClip", tb.getNextHighestDepth());
	var actorNameMaxWidth = 270 - tb.ActorName._x;
	actorNameMaskClip.beginFill(0xFF0000);
	actorNameMaskClip.moveTo(0, 0);
	actorNameMaskClip.lineTo(actorNameMaxWidth, 0);
	actorNameMaskClip.lineTo(actorNameMaxWidth, tb.ActorName._height);
	actorNameMaskClip.lineTo(0, tb.ActorName._height);
	actorNameMaskClip.lineTo(0, 0);
	actorNameMaskClip.endFill();
	actorNameMaskClip._visible = true;
	actorNameMaskClip._x = tb.ActorName._x;
	actorNameMaskClip._y = tb.ActorName._y;
	tb.ActorName.setMask(actorNameMaskClip);
	var startDelay:Number = -1;
	var endDelay:Number = -1;
	var startTime:Number = -1;
	function scrollText(){
		if (tb.ActorName._width <= actorNameMaxWidth){
			return;
		}
		var currentTime:Number = getTimer();
		if (startTime == -1){
			startTime = currentTime;
		}
		if (startDelay < SCROLL_DELAY){
			startDelay = (currentTime - startTime) / 1000;
		}
		else if(startDelay >= SCROLL_DELAY && startDelay != 123456789){
			startDelay = 123456789;
			startTime = currentTime;
		}
		else if (-tb.ActorName.Text._x + actorNameMaxWidth < tb.ActorName.Text._width){
			tb.ActorName.Text._x = -((currentTime - startTime) / (SCROLL_SPEED * 0.5));
		}
		else if(startDelay >= SCROLL_DELAY && endDelay == -1){
			startTime = currentTime;
			endDelay = (currentTime - startTime) / 1000;
		}
		else if(endDelay < SCROLL_DELAY){
			endDelay = (currentTime - startTime) / 1000;
		}
		else{
			tb.ActorName.Text._x = 0;
			startTime = currentTime;
			endDelay = -1;
			startDelay = -1;
		}
	}
	tb.ActorName.onEnterFrame = scrollText;
	tb.ActorName.Text._x = 0;
}

function enableAutoSize(instanceS:MovieClip){
	var tb = instanceS.TextBoxes;
	tb.ActorLabel.Text._width = 1;
	tb.ActorLabel.Text.autoSize = true;
	tb.ActorName._x = tb.ActorLabel._x + tb.ActorLabel.Text._width + diffOffset;
	tb.SlotLabel.Text._width = 1;
	tb.SlotLabel.Text.autoSize = true;
	tb.SlotNumbers.Text.autoSize = true;
	tb.TypeLabel.Text._width = 1;
	tb.TypeLabel.Text.autoSize = true;
	tb.ArmorType.Text.autoSize = true;
	tb.DamageDiff.Text.autoSize = true;
	tb.RatingLabel.Text._width = 1;
	tb.RatingLabel.Text.autoSize = true;
	tb.ArmorRating.Text._width = 1;
	tb.ArmorRating.Text.autoSize = true;
	tb.RatingDiff.Text.autoSize = true;
	tb.GoldLabel.Text._width = 1;
	tb.GoldLabel.Text.autoSize = true;
	tb.GoldValue.Text.autoSize = true;
	tb.ValueDiff.Text.autoSize = true;
	tb.SpeedLabel.Text.autoSize = true;
	tb.Speed.Text.autoSize = true;
	tb.SpeedDiff.Text.autoSize = true;
	tb.StaggerLabel.Text.autoSize = true;
	tb.Stagger.Text.autoSize = true;
	tb.StaggerDiff.Text.autoSize = true;
	tb.ReachLabel.Text.autoSize = true;
	tb.Reach.Text.autoSize = true;
	tb.ReachDiff.Text.autoSize = true;
	tb.EffectsLabel.Text.autoSize = true;
}

function offsetValuesAndDiffs(instanceO:MovieClip){
	var tb = instanceO.TextBoxes;
	
	//LEFT
	var arrayOfLeftWidths:Array = new Array(
		tb.SlotLabel.Text._width,
		tb.TypeLabel.Text._width,
		tb.RatingLabel.Text._width,
		tb.GoldLabel.Text._width
		)
		
	var arrayOfLeftValues:Array = new Array(
		tb.SlotNumbers,
		tb.ArmorType,
		tb.ArmorRating,
		tb.GoldValue
		)
	


	var arrayOfLeftDiffs:Array = new Array(
		tb.DamageDiff,
		tb.RatingDiff,
		tb.ValueDiff
		)
	//Get max width of left labels
	var leftMax:Number = 0;
	for (var i:Number = 0; i < arrayOfLeftWidths.length; i++){
		leftMax = Math.max(leftMax, arrayOfLeftWidths[i]);
	}
	//Set X of left values based on left max label width + value offset
	for (var i:Number = 0; i < arrayOfLeftValues.length; i++) {
		var field:MovieClip = arrayOfLeftValues[i];
		field._x = tb.SlotLabel._x + leftMax + valueOffset;
	}
	//Set left Diff Xs based on their value X + value width + diff offset
	for (var i:Number = 0; i < arrayOfLeftDiffs.length; i++){
		var diff:MovieClip = arrayOfLeftDiffs[i];
		var val:MovieClip = arrayOfLeftValues[i+1];
		diff._x = val._x + val.Text._width + diffOffset;
	}
	
	//RIGHT
	var arrayOfRightLabels:Array = new Array(
		tb.SpeedLabel,
		tb.StaggerLabel,
		tb.ReachLabel
		)
		
	var arrayOfRightValues:Array = new Array(
		tb.Speed,
		tb.Stagger,
		tb.Reach
		)
	
	var arrayOfRightDiffs:Array = new Array(
		tb.SpeedDiff,
		tb.StaggerDiff,
		tb.ReachDiff
		)
	//Set X of right labels based on columTwoOffset
	for (var i:Number = 0; i < arrayOfRightLabels.length; i++) {
		var field:MovieClip = arrayOfRightLabels[i];
		field._x = columnTwoOffset;
	}
	//Get max width of Right Labels
	var rightMax:Number = 0;
	for (var i:Number = 0; i < arrayOfRightLabels.length; i++){
		var field:MovieClip = arrayOfRightLabels[i];
		rightMax = Math.max(rightMax, field.Text._width);
	}
	//Set X of right values based on left max label width + value offset
	for (var i:Number = 0; i < arrayOfRightValues.length; i++) {
		var field:MovieClip = arrayOfRightValues[i];
		field._x = tb.SpeedLabel._x + rightMax + valueOffset;
	}
	//Set right Diff Xs based on their value X + value width + diff offset
	for (var i:Number = 0; i < arrayOfRightDiffs.length; i++){
		var diff:MovieClip = arrayOfRightDiffs[i];
		var val:MovieClip = arrayOfRightValues[i];
		diff._x = val._x + val.Text._width + diffOffset;
	}
	actorNameScroller(tb);
	itemNameScroller(tb);
	effectsScroller(tb);
}

function populateSelectedArmorItemCard(){
	setArmorLabels(_root.ce_assets.SelectedItemCard);
	enableAutoSize(_root.ce_assets.SelectedItemCard);
	var tb = _root.ce_assets.SelectedItemCard.TextBoxes;
	tb.ActorName.Text.htmlText = normalWrap(actor);
	var i = 0;
	tb.ItemName.Text.htmlText = normalWrap(arguments[i++]);
	tb.SlotNumbers.Text.htmlText = normalWrap(arguments[i++]);
	tb.ArmorType.Text.htmlText = normalWrap(arguments[i++]);
	tb.ArmorRating.Text.htmlText = normalWrap(arguments[i++]);
	var ratingDifference = parseInt(arguments[i++]);
	tb.GoldValue.Text.htmlText = normalWrap(String(parseInt(arguments[i++])));
	var valueDifference = parseInt(arguments[i++]);
	tb.Effects.Text.htmlText = normalWrap(arguments[i++]);
	if (ratingDifference != 0){
		tb.RatingDiff._visible = true;
		if (ratingDifference > 0) {
			tb.RatingDiff.Text.htmlText = betterWrap(ratingDifference);
		}
		else{
			tb.RatingDiff.Text.htmlText = worseWrap(ratingDifference);
		}
	}
	else{
		tb.RatingDiff._visible = false;
		tb.RatingDiff.Text.htmlText = 0;
	}
	if (valueDifference != 0){
		tb.ValueDiff._visible = true;
		if (valueDifference > 0){
			tb.ValueDiff.Text.htmlText = betterWrap(valueDifference);
		}
		else{
			tb.ValueDiff.Text.htmlText = worseWrap(valueDifference);
		}
	}
	tb.DamageDiff._visible = false;
	tb.CritDiff._visible = false;
	tb.StaggerLabel._visible = false;
	tb.Stagger._visible = false;
	tb.StaggerDiff._visible = false;
	tb.SpeedLabel._visible = false;
	tb.Speed._visible = false;
	tb.SpeedDiff._visible = false;
	tb.ReachLabel._visible = false;
	tb.Reach._visible = false;
	tb.ReachDiff._visible = false;
	var entryObject:Object = arguments[i++];
	if (entryObject.iconLabel != undefined){
		skse.plugins.InventoryInjector.ProcessEntry(entryObject);
		loadIcon(_root.ce_assets.SelectedItemCard, entryObject.iconLabel, entryObject.iconSource, entryObject.iconColor);
	}
	else{
		loadIcon(_root.ce_assets.SelectedItemCard, DEFAULT_ICON_LABEL, DEFAULT_ICON_SOURCE, DEFAULT_ICON_COLOR);
	}
}

function populateSelectedWeaponItemCard(){
	_root.ce_assets.SelectedItemCard._visible = false;
	var newInstance:MovieClip;
	//Up to two selected item cards, one for each equipped hand.
	newInstance = _root.ce_assets.attachMovie("ItemCard", "instanceSelected_" + selectedCounter, _root.ce_assets.getNextHighestDepth());
	selectedCounter++;
	newInstance.Background._alpha = ALPHA;
	setWeaponLabels(newInstance);
	var tb = newInstance.TextBoxes;
	tb.ActorLabel.Text.htmlText = comparingTo;
	tb.ActorName.Text.htmlText = normalWrap(actor);
	var i = 0;
	tb.ItemName.Text.htmlText = normalWrap(arguments[i++]);
	tb.SlotNumbers.Text.htmlText = notApplicable;
	
	//Damage section
	tb.ArmorType.Text.htmlText = normalWrap(arguments[i++]);
	var damageDifference = precisionTwo(arguments[i++]);
	if (damageDifference != 0){
		tb.DamageDiff._visible = true;
		if (damageDifference > 0) {
			tb.DamageDiff.Text.htmlText = betterWrap(damageDifference);
			tb.DamageDiff.better = true;
		}
		else{
			tb.DamageDiff.Text.htmlText = worseWrap(damageDifference);
			tb.DamageDiff.better = false;
		}
	}
	else{
		tb.DamageDiff._visible = false;
		tb.DamageDiff.Text.htmlText = "()";
	}
	
	//Crit section
	tb.ArmorRating.Text.htmlText = normalWrap(precisionTwo(arguments[i++]));
	var critDifference = precisionTwo(arguments[i++]);
	if (critDifference != 0){
		tb.RatingDiff._visible = true;
		if (critDifference > 0) {
			tb.RatingDiff.Text.htmlText = betterWrap(critDifference);
			tb.RatingDiff.better = true;
		}
		else{
			tb.RatingDiff.Text.htmlText = worseWrap(critDifference);
			tb.RatingDiff.better = false;
		}
	}
	else{
		tb.RatingDiff._visible = false;
		tb.RatingDiff.Text.htmlText = "()";
	}
	
	//Gold section
	tb.GoldValue.Text.htmlText = normalWrap(arguments[i++]);
	var valueDifference = arguments[i++];
	if (valueDifference != 0.0){
		tb.ValueDiff._visible = true;
		if (valueDifference > 0){
			tb.ValueDiff.Text.htmlText = betterWrap(valueDifference);
			tb.ValueDiff.better = true;
		}
		else{
			tb.ValueDiff.Text.htmlText = worseWrap(valueDifference);
			tb.ValueDiff.better = false;
		}
	}
	else{
		tb.ValueDiff._visible = false;
		tb.ValueDiff.Text.htmlText = "()";
	}
	
	//Speed section
	tb.SpeedLabel._visible = true;
	tb.Speed._visible = true;
	tb.Speed.Text.htmlText = normalWrap(precisionTwo(arguments[i++]));
	var speedDifference = precisionTwo(arguments[i++]);
	if (speedDifference != 0.0){
		tb.SpeedDiff._visible = true;
		if (speedDifference > 0) {
			tb.SpeedDiff.Text.htmlText = betterWrap(speedDifference);
			tb.SpeedDiff.better = true;
		}
		else{
			tb.SpeedDiff.Text.htmlText = worseWrap(speedDifference);
			tb.SpeedDiff.better = false;
		}
	}
	else{
		tb.SpeedDiff._visible = false;
		tb.SpeedDiff.Text.htmlText = "()";
	}
	
	//Stagger section
	tb.StaggerLabel._visible = true;
	tb.Stagger._visible = true;
	tb.Stagger.Text.htmlText = normalWrap(precisionTwo(arguments[i++]));
	var staggerDifference = precisionTwo(arguments[i++]);
	if (staggerDifference != 0.0){
		tb.StaggerDiff._visible = true;
		if (staggerDifference > 0) {
			tb.StaggerDiff.Text.htmlText = betterWrap(staggerDifference);
			tb.StaggerDiff.better = true;
		}
		else{
			tb.StaggerDiff.Text.htmlText = worseWrap(staggerDifference);
			tb.StaggerDiff.better = false;
		}
	}
	else{
		tb.StaggerDiff._visible = false;
		tb.StaggerDiff.Text.htmlText = "()";
	}
	
	//Reach section
	tb.ReachLabel._visible = true;
	tb.Reach._visible = true;
	tb.Reach.Text.htmlText = normalWrap(precisionTwo(arguments[i++]));
	var reachDifference = precisionTwo(arguments[i++]);
	if (reachDifference != 0.0){
		tb.ReachDiff._visible = true;
		if (reachDifference > 0) {
			tb.ReachDiff.Text.htmlText = betterWrap(reachDifference);
			tb.ReachDiff.better = true;
		}
		else{
			tb.ReachDiff.Text.htmlText = worseWrap(reachDifference);
			tb.ReachDiff.better = false;
		}
	}
	else{
		tb.ReachDiff._visible = false;
		tb.ReachDiff.Text.htmlText = "()";
	}
	tb.Effects.Text.htmlText = normalWrap(arguments[i++]);
	enableAutoSize(newInstance);
	var entryObject:Object = arguments[i++];
	if (entryObject.iconLabel != undefined){
		skse.plugins.InventoryInjector.ProcessEntry(entryObject);
		loadIcon(newInstance, entryObject.iconLabel, entryObject.iconSource, entryObject.iconColor);
	}
	else{
		loadIcon(newInstance, DEFAULT_ICON_LABEL, DEFAULT_ICON_SOURCE, DEFAULT_ICON_COLOR);
	}
}

function reset(){
	var i:Number;
	var instanceToRemove;
	for (i = 0; i < counter; i++){
		instanceToRemove = _root.ce_assets["instance_" + i];
		instanceToRemove.removeMovieClip();
	}
	for (i = 0; i < selectedCounter; i++){
		instanceToRemove = _root.ce_assets["instanceSelected_" + i];
		instanceToRemove.removeMovieClip();
	}
	counter = 0;
	selectedCounter = 0;
	_root.ce_assets.SelectedItemCard.Background._alpha = ALPHA;
	var tb = _root.ce_assets.SelectedItemCard.TextBoxes;
	tb.ActorLabel.Text.htmlText = comparingTo;
	tb.ActorName.Text.htmlText = noneText;
	tb.ItemName.Text.htmlText = noneText;
	tb.SlotNumbers.Text.htmlText = noneText;
	tb.ArmorType.Text.htmlText = noneText;
	tb.DamageDiff._visible = false;
	tb.DamageDiff.Text.htmlText = "()";
	
	
	tb.ArmorRating.Text.htmlText = "0";
	tb.RatingDiff.Text.htmlText = "()";
	tb.RatingDiff._visible = false;
	
	tb.GoldValue.Text.htmlText = "0";
	tb.ValueDiff.Text.htmlText = "()";
	tb.ValueDiff._visible = false;
	
	tb.SpeedLabel._visible = false;
	tb.Speed._visible = false;
	tb.SpeedDiff._visible = false;
	tb.Speed.Text.htmlText = "0";
	tb.SpeedDiff.Text.htmlText = "()";
	
	
	tb.StaggerLabel._visible = false;
	tb.Stagger._visible = false;
	tb.StaggerDiff._visible = false;
	tb.Stagger.Text.htmlText = noneText;
	tb.StaggerDiff._visible = false;


	tb.ReachLabel._visible = false;

	tb.Reach._visible = false;
	tb.ReachDiff._visible = false;
	tb.Reach.Text.htmlText = "0";
	tb.ReachDiff.Text.htmlText = "()";
	
	tb.Effects.Text.htmlText = noneText;
}



function createArmorComparisonItemCard(){
	if (arguments[0] == "DISPLAY"){
		positionItemCards();
		return;
	}
	var newInstance:MovieClip;
	newInstance = _root.ce_assets.attachMovie("ItemCard", "instance_" + counter, _root.ce_assets.getNextHighestDepth());
	newInstance.Background._alpha = ALPHA;
	setArmorLabels(newInstance);
	var tb = newInstance.TextBoxes;
	tb.ActorLabel.Text.htmlText = equippedTo;
	tb.ActorName.Text.htmlText = normalWrap(actor);
	var i = 0;
	tb.ItemName.Text.htmlText = normalWrap(arguments[i++]);
	tb.SlotNumbers.Text.htmlText = normalWrap(arguments[i++]);
	tb.ArmorType.Text.htmlText = normalWrap(arguments[i++]);
	tb.ArmorRating.Text.htmlText = normalWrap(arguments[i++]);
	tb.GoldValue.Text.htmlText = normalWrap(String(parseInt(arguments[i++])));
	tb.Effects.Text.htmlText = normalWrap(arguments[i++]);
	
	tb.DamageDiff._visible = false;
	tb.CritDiff._visible = false;
	tb.ValueDiff._visible = false;
	tb.RatingDiff._visible = false;
	tb.StaggerLabel._visible = false;
	tb.Stagger._visible = false;
	tb.StaggerDiff._visible = false;
	tb.SpeedLabel._visible = false;
	tb.Speed._visible = false;
	tb.SpeedDiff._visible = false;
	tb.ReachLabel._visible = false;
	tb.Reach._visible = false;
	tb.ReachDiff._visible = false;
	counter++;
	enableAutoSize(newInstance);
	var entryObject:Object = arguments[i++];
	if (entryObject.iconLabel != undefined){
		skse.plugins.InventoryInjector.ProcessEntry(entryObject);
		loadIcon(newInstance, entryObject.iconLabel, entryObject.iconSource, entryObject.iconColor);
	}
	else{
		loadIcon(newInstance, DEFAULT_ICON_LABEL, DEFAULT_ICON_SOURCE, DEFAULT_ICON_COLOR);
	}
}

function createWeaponComparisonItemCard(){
	if (arguments[0] == "DISPLAY"){
		positionItemCards();
		return;
	}
	var newInstance:MovieClip;
	var i = 0;
	newInstance = _root.ce_assets.attachMovie("ItemCard", "instance_" + counter, _root.ce_assets.getNextHighestDepth());
	newInstance.Background._alpha = ALPHA;
	setWeaponLabels(newInstance);
	var tb = newInstance.TextBoxes;
	
	tb.ActorLabel.Text.htmlText = equippedTo;
	tb.ActorName.Text.htmlText = normalWrap(actor);
	
	tb.SlotNumbers.Text.htmlText = normalWrap(arguments[i++]);
	
	tb.ItemName.Text.htmlText = normalWrap(arguments[i++]);
	
	tb.ArmorType.Text.htmlText = normalWrap(arguments[i++]);
	
	tb.ArmorRating.Text.htmlText = normalWrap(precisionTwo(arguments[i++]));
	
	tb.GoldValue.Text.htmlText = normalWrap(arguments[i++]);
	
	tb.Speed.Text.htmlText = normalWrap(precisionTwo(arguments[i++]));
	
	tb.Stagger.Text.htmlText = normalWrap(precisionTwo(arguments[i++]));
	
	tb.Reach.Text.htmlText = normalWrap(precisionTwo(arguments[i++]));
	
	tb.Effects.Text.htmlText = normalWrap(arguments[i++]);
	
	tb.DamageDiff._visible = false;
	tb.CritDiff._visible = false;
	tb.RatingDiff._visible = false;
	tb.ValueDiff._visible = false;
	tb.SpeedDiff._visible = false;
	tb.StaggerDiff._visible = false;
	tb.ReachDiff._visible = false;
	counter++;
	enableAutoSize(newInstance);
	var entryObject:Object = arguments[i++];
	if (entryObject.iconLabel != undefined){
		skse.plugins.InventoryInjector.ProcessEntry(entryObject);
		loadIcon(newInstance, entryObject.iconLabel, entryObject.iconSource, entryObject.iconColor);
	}
	else{
		loadIcon(newInstance, DEFAULT_ICON_LABEL, DEFAULT_ICON_SOURCE, DEFAULT_ICON_COLOR);
	}
}

function positionItemCards(){
	_root.ce_assets._visible = true;
	var i:Number = 0;
	var instanceToPosition;
	var scale:Number = (counter > 4) ? 100 * (3.3 / 4) : 100;
	var itemHeight:Number = (_root.ce_assets.SelectedItemCard.Background._height * (scale / 100)) + spacingBetweenEquippedY;
	offsetValuesAndDiffs(_root.ce_assets.SelectedItemCard);
	if (selectedCounter == 0){
		_root.ce_assets.SelectedItemCard._visible = true;
	}
	for (i = 0; i < counter; i++) {
		instanceToPosition = _root.ce_assets["instance_" + i];
		if (counter >= 4)
		{
			instanceToPosition._xscale = scale;
			instanceToPosition._yscale = scale;
		}
		var col:Number = Math.floor(i / maxRows);
		var row:Number = i % maxRows;
		var itemsInCol:Number = Math.min(maxRows, counter - col * maxRows);
		var totalHeight:Number = (itemsInCol * itemHeight) - spacingBetweenEquippedY;
		var startY:Number = 0;
		if (layout == 0){
			startY = _root.ce_assets.SelectedItemCard._y - (totalHeight / 2) + (instanceToPosition.Background._height / 2);
			instanceToPosition._y = startY + row * itemHeight;
		}
		else if (layout == 1){
			startY = _root.ce_assets.SelectedItemCard._y;
			instanceToPosition._y = startY - row * itemHeight;
		}
		else if (layout == 2){
			startY = _root.ce_assets.SelectedItemCard._y;
			instanceToPosition._y = startY + row * itemHeight;
		}
		instanceToPosition._x = _root.ce_assets.SelectedItemCard.Background._width + spacingFromSelected + col * (instanceToPosition.Background._width + spacingBetweenEquippedX);
		instanceToPosition.Background._alpha = ALPHA;
		offsetValuesAndDiffs(instanceToPosition);
	}
	for (i = 0; i < selectedCounter; i++) {
		instanceToPosition = _root.ce_assets["instanceSelected_" + i];
		var col:Number = 1;
		var row:Number = i % selectedCounter;
		var totalHeight:Number = (selectedCounter * itemHeight) - 5;
		var startY:Number = 0;
		if (layout == 0){
			startY = _root.ce_assets.SelectedItemCard._y - (totalHeight / 2) + (instanceToPosition.Background._height / 2);
			instanceToPosition._y = startY + row * itemHeight;
		}
		else if (layout == 1){
			startY = _root.ce_assets.SelectedItemCard._y;
			instanceToPosition._y = startY - row * itemHeight;
		}
		else if (layout == 2){
			startY = _root.ce_assets.SelectedItemCard._y;
			instanceToPosition._y = startY + row * itemHeight;
		}
		instanceToPosition._x = _root.ce_assets.SelectedItemCard._x;
		instanceToPosition.Background._alpha = ALPHA;
		if (i == 0 && selectedCounter == 1 && counter > 1){
			var tb = instanceToPosition.TextBoxes;
			tb.SlotLabel.Text.htmlText = normalWrap(keyText);
			tb.SlotNumbers.Text.htmlText = normalWrap(keyInfo);
			tb.ValueDiff.Text.htmlText = tb.ValueDiff.Text.htmlText + GetKeyText(totalKey, tb.ValueDiff.Text.htmlText);
			tb.DamageDiff.Text.htmlText = tb.DamageDiff.Text.htmlText + GetKeyText(averageKey, tb.DamageDiff.Text.htmlText);
			tb.RatingDiff.Text.htmlText = tb.RatingDiff.Text.htmlText + GetKeyText(averageKey, tb.RatingDiff.Text.htmlText);
			tb.SpeedDiff.Text.htmlText = tb.SpeedDiff.Text.htmlText + GetKeyText(averageKey, tb.SpeedDiff.Text.htmlText);
			tb.StaggerDiff.Text.htmlText = tb.StaggerDiff.Text.htmlText + GetKeyText(averageKey, tb.StaggerDiff.Text.htmlText);
			tb.ReachDiff.Text.htmlText = tb.ReachDiff.Text.htmlText + GetKeyText(maxKey, tb.ReachDiff.Text.htmlText);
		}
		offsetValuesAndDiffs(instanceToPosition);
	}
}

function getColorFromHTML(htmlText:String):String {
	var colorIndex:Number = htmlText.indexOf("COLOR=");
	if (colorIndex == -1)
		return "#FFFFFF";
		
	var afterColor:String = htmlText.substr(colorIndex + 7);
	var endIndex:Number = afterColor.indexOf('"');
	var colorValue:String = afterColor.substr(0, endIndex);
	return colorValue;
}

function GetKeyText(str:String, htmlText:String){
	var tColor:String = getColorFromHTML(htmlText);
	if (str.substr(0, 1) == ";"){
		return "<font color=\"" + tColor + "\"> " + str.substr(1) + "</font>";
	}
	return "<font color=\"" + tColor + "\">"+ str + "</font>";
}

function setActor(new_actor:String){
	actor = new_actor;
}

function showMenu(){
	_root.ce_assets._visible = true;
}

function getVisible(){
	return _root.ce_assets._visible;
}

function SetXYOffsets(x:Number, y:Number){
	_root.ce_assets._x = x;
	_root.ce_assets._y = y;
}

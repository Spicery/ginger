PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE "SimpleKey" (
    `Name`  TEXT,
    `KeyID` INTEGER,
    `NumFields` INTEGER,
    `Sublayout` INTEGER,
    `Layout`    INTEGER,
    `HiTag` INTEGER,
    `LoTag` INTEGER,
     FOREIGN KEY (Layout) REFERENCES Layout(Value)
);
INSERT INTO "SimpleKey" VALUES('AbsentKey',0,0,0,0,4,3);
INSERT INTO "SimpleKey" VALUES('BooleanKey',1,0,0,0,4,3);
INSERT INTO "SimpleKey" VALUES('KeyKey',2,4,0,1,4,3);
INSERT INTO "SimpleKey" VALUES('TerminKey',3,0,0,0,4,3);
INSERT INTO "SimpleKey" VALUES('NilKey',4,0,0,0,4,3);
INSERT INTO "SimpleKey" VALUES('PairKey',5,2,1,0,4,3);
INSERT INTO "SimpleKey" VALUES('VectorKey',6,0,0,2,4,3);
INSERT INTO "SimpleKey" VALUES('StringKey',7,0,0,3,4,3);
INSERT INTO "SimpleKey" VALUES('WordKey (Not used???)',8,1,0,0,4,3);
INSERT INTO "SimpleKey" VALUES('SmallKey',9,0,0,0,4,3);
INSERT INTO "SimpleKey" VALUES('FloatKey',10,0,0,0,4,3);
INSERT INTO "SimpleKey" VALUES('UnicodeKey',11,0,0,0,4,3);
INSERT INTO "SimpleKey" VALUES('CharKey',12,0,0,0,4,3);
INSERT INTO "SimpleKey" VALUES('MapletKey',13,2,0,1,4,3);
INSERT INTO "SimpleKey" VALUES('IndirectionKey',14,1,0,1,4,3);
INSERT INTO "SimpleKey" VALUES('AssocKey',15,3,0,1,4,3);
CREATE TABLE "Layout" (
	`Name`	TEXT,
	`Value`	INTEGER,
	`Comment`	TEXT,
	PRIMARY KEY(`Value`)
);
INSERT INTO "Layout" VALUES('Keyless_Layout',0,'Keys with this layout are used for primitive values i.e. do not correspond to a heap object.');
INSERT INTO "Layout" VALUES('Record_Layout',1,'Keys with this layout are used for reference values that have a fixed numner of fields. In this case both the sublayout and numfields columns will be in use.');
INSERT INTO "Layout" VALUES('Vector_Layout',2,'Keys with this layout are used for reference values that have a variable number of full-word fields and use a length field immediately ahead of the key. The length field is in SmallInt format in order to meet the constraint that fields ahead of the key are guaranteed to not be confusable with a key.');
INSERT INTO "Layout" VALUES('String_Layout',3,'Keys with this layout are used for reference values that have a variable number of byte width fields and use a length field immediately ahead of the key. The length field is in SmallInt format in order to meet the constraint that fields ahead of the key are guaranteed to not be confusable with a key.');
INSERT INTO "Layout" VALUES('Mixed_Layout',4,'');
INSERT INTO "Layout" VALUES('WRecord_Layout',5,NULL);
CREATE TABLE `Sublayout` ( 
    `Name` TEXT, 
    `LayoutValue` INTEGER, 
    `SublayoutValue` INTEGER, 
    `Comment` TEXT, 
    PRIMARY KEY(`Name`) 
    FOREIGN KEY (LayoutValue) REFERENCES Layout(Value) 
);
CREATE TABLE `About.Table` (
	`TableName`	TEXT,
	`Comment`	TEXT
);
INSERT INTO "About.Table" VALUES('SimpleKey','This table records the details of simple-keys.');
CREATE TABLE "About.Column" (
	`TableName`	TEXT,
	`ColumnName`	TEXT,
	`Comment`	TEXT
);
INSERT INTO "About.Column" VALUES('SimpleKey','Name','Name of the key.');
INSERT INTO "About.Column" VALUES('SimpleKey','KeyID','Unique ID for each simple key. 16 bits from 16-31.');
INSERT INTO "About.Column" VALUES('SimpleKey','NumFields','A count of the fixed fields of objects of this type.');
INSERT INTO "About.Column" VALUES('SimpleKey','Sublayout','A 4 bit field that refines in the info in the layout.');
INSERT INTO "About.Column" VALUES('SimpleKey','Layout','A 3 bit field that categorises how values of this type are laid out in the heap.');
INSERT INTO "About.Column" VALUES('SimpleKey','HiTag','3 bit tag');
INSERT INTO "About.Column" VALUES('SimpleKey','LoTag','2 bit tag');
CREATE UNIQUE INDEX Layout_Name on Layout( Name )

;
COMMIT;

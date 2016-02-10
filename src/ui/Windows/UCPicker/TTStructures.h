
#pragma once

struct st_cmapindex {
  UINT16 version;
  UINT16 numsubtables;
};

struct st_subtable_header {
  UINT16 platformID;
  UINT16 platformSpecificID;
  UINT32 offset;
};

struct st_format_header {
  UINT16 format;
  UINT16 length;
  UINT16 language;
};

struct st_header_table {
  UINT32 version;
  UINT32 fontRevision;
  UINT32 checkSumAdjustment;
  UINT32 magicNumber;
  UINT16 flags;
  UINT16 unitsPerEm;
  INT64 created;
  INT64 modified;
  INT16 xMin;
  INT16 yMin;
  INT16 xMax;
  INT16 yMax;
  UINT16 macStyle;
  UINT16 lowestRecPPEM;
  INT16 fontDirectionHint;
  INT16 indexToLocFormat;
  INT16 glyphDataFormat;
};

struct st_format4 {
  UINT16 	format;
  UINT16 	length;
  UINT16 	language;
  UINT16 	segCountX2;
  UINT16 	searchRange;
  UINT16 	entrySelector;
  UINT16 	rangeShift;
  UINT16 	endCode; // [segCount]
  UINT16 	reservedPad;
  UINT16 	startCode; // [segCount]
  UINT16 	idDelta; // [segCount]
  UINT16 	idRangeOffset; // [segCount]
  UINT16 	glyphIndexArray;
};

struct st_format12 {
  UINT16 format;
  UINT16 reserved;
  UINT32 length;
  UINT32 language;
  UINT32 nGroups;
};

struct st_group {
  UINT32 startCharCode;
  UINT32 endCharCode;
  UINT32 startGlyphCode;
};

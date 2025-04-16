void
parseSWF_GLYPHENTRY (FILE * f, SWF_GLYPHENTRY *gerec, int glyphbits, int advancebits)
{
  unsigned int i;

  size_t nmalloc = ( glyphbits < 1 ? 1 : ((glyphbits+31)/32) ) * sizeof(UI32);
  gerec->GlyphIndex = malloc(nmalloc);
  gerec->GlyphIndex[0] = 0; /* for glyphbits == 0 */
  for( i=0; glyphbits; i++ ) {
    if (i < (nmalloc / sizeof(UI32))) {
	  if( glyphbits > 32 ) {
	  	gerec->GlyphIndex[i] = readBits(f, 32);
	  	glyphbits -= 32;
  	} else {
	 	gerec->GlyphIndex[i] = readBits(f, glyphbits);
	 	glyphbits = 0;
  	}
    } else {
      SWF_error("unexpected end of file");
    }
  }

  nmalloc = ( advancebits < 1 ? 1 : ((advancebits+31)/32) ) * sizeof(UI32);
  gerec->GlyphAdvance = malloc(nmalloc);
  gerec->GlyphAdvance[0] = 0; /* for advancebits == 0 */
  for( i=0; advancebits; i++ ) {
    if (i < (nmalloc / sizeof(UI32))) {
	  if( advancebits > 32 ) {
	  	gerec->GlyphAdvance[i] = readBits(f, 32);
	  	advancebits -= 32;
  	} else {
	 	gerec->GlyphAdvance[i] = readBits(f, advancebits);
	 	advancebits = 0;
  	}
    } else {
      SWF_error("unexpected end of file");
    }
  }
}
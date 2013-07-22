<?xml version="1.0" encoding="UTF-8" ?> 

<xsl:stylesheet version="1.0"
 xmlns:xsl="http://www.w3.org/1999/XSL/Transform">


<xsl:strip-space elements="*"/>

<xsl:template match="/worksheet">
<!--  <xsl:message terminate="no">progress:worksheet</xsl:message> -->
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="sheetData">
<!--  <xsl:message terminate="no">progress:sheetData</xsl:message> -->
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="row">
<!--  <xsl:message terminate="no">progress:row</xsl:message> -->
  <p>
    <xsl:apply-templates select="c" />
  </p>
</xsl:template>

<xsl:template match="c">
<!--  <xsl:message terminate="no">progress:c</xsl:message> -->
  <span>
    <xsl:value-of select="is | v" />
  </span>
</xsl:template>

</xsl:stylesheet> 

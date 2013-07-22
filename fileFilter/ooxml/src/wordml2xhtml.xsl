<?xml version="1.0" encoding="UTF-8" ?> 

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:pchar="urn:cleverage:xmlns:post-processings:characters"
  xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0"
  xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
  xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
  xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
  xmlns:number="urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0"
  xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
  xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
  xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:dc="http://purl.org/dc/elements/1.1/"
  xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
  xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
  xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
  xmlns:pcut="urn:cleverage:xmlns:post-processings:pcut"
  xmlns:v="urn:schemas-microsoft-com:vml" 
  xmlns:o="urn:schemas-microsoft-com:office:office"
  xmlns:oox="urn:oox"
  exclude-result-prefixes="pchar draw office fo style number text w r xlink number wp svg pcut dc v o oox">

<xsl:strip-space elements="*"/>
<xsl:preserve-space elements="w:p"/>
<xsl:preserve-space elements="w:r"/>

<xsl:template match="w:document">
<!--  <xsl:message terminate="no">progress:w:document</xsl:message> -->
  <html>
    <xsl:apply-templates/>
  </html>
</xsl:template>

<xsl:template match="w:body">
<!--  <xsl:message terminate="no">progress:w:body</xsl:message> -->
  <body>
    <xsl:apply-templates/>
  </body>
</xsl:template>

<xsl:template match="w:p">
<!--  <xsl:message terminate="no">progress:w:p</xsl:message> -->
  <p>
    <xsl:apply-templates select="w:r | w:hlink" />
  </p>
</xsl:template>

<xsl:template match="w:r">
  <xsl:choose>
    <xsl:when test="w:rPr/w:i">
      <em>
        <xsl:value-of select="w:t" />
      </em>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="w:t" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="w:hlink">
  <a href="{@w:dest}">
    <xsl:apply-templates select="w:r" />
  </a>
</xsl:template>

</xsl:stylesheet> 

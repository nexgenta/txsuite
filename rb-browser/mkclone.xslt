<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		xmlns:xsd="http://www.w3.org/2001/XMLSchema">
<xsl:output method="text"/>

<!-- the TYPE parameter should be the name of the top level type -->
<xsl:param name="TYPE" select="'TYPE parameter not defined'"/>

<xsl:template match="xsd:schema">
void
<xsl:value-of select="$TYPE"/>_dup(<xsl:value-of select="$TYPE"/> *dst, <xsl:value-of select="$TYPE"/> *src)
{
	<xsl:apply-templates select="xsd:complexType[@name=$TYPE]"/>
	<xsl:apply-templates select="xsd:simpleType[@name=$TYPE]"/>
	return;
}
</xsl:template>

<!-- aliases -->
<xsl:template match="xsd:simpleType/xsd:restriction | xsd:complexType/xsd:complexContent/xsd:extension">
	<!-- see what type we are dealing with -->
	<xsl:choose>
		<!-- a boolean or an integer or an enum, just use = to copy the values -->
		<xsl:when test="@base='xsd:boolean' or @base='xsd:integer' or @base='xsd:token'">
	*dst = *src;
		</xsl:when>
		<!-- xsd:hexBinary is an OctetString -->
		<xsl:when test="@base='xsd:hexBinary'">
	OctetString_dup(dst, src);
		</xsl:when>
		<!-- a complex type, use TYPE_dup() to copy -->
		<xsl:otherwise>
	<xsl:value-of select="@base"/>_dup(dst, src);
		</xsl:otherwise>
	</xsl:choose>
</xsl:template>

<!-- CHOICE types -->
<xsl:template match="xsd:complexType/xsd:choice">
	/* CHOICE */
	dst->choice = src->choice;

	switch(src->choice)
	{
	<xsl:apply-templates select="xsd:element"/>
	default:
		error("Unknown <xsl:value-of select="$TYPE"/> type: %d", src->choice);
		break;
	}
</xsl:template>

<xsl:template match="xsd:choice/xsd:element">
	<!-- see what type we are dealing with -->
	<!-- need to handle the case when @type does not exist, eg internal_reference in ObjectReference -->
	<!-- in this case, the type is an integer -->
	<xsl:choose>
		<!-- a boolean or an integer, just use = to copy the values -->
		<xsl:when test="string(@type)='' or @type='xsd:boolean' or @type='xsd:integer'">
	case <xsl:value-of select="$TYPE"/>_<xsl:value-of select="@name"/>:
		dst->u.<xsl:value-of select="@name"/> = src->u.<xsl:value-of select="@name"/>;
		break;
		</xsl:when>
		<!-- a hexBinary, use OctetString_dup() to copy the values -->
		<xsl:when test="@type='xsd:hexBinary'">
	case <xsl:value-of select="$TYPE"/>_<xsl:value-of select="@name"/>:
		OctetString_dup(&amp;dst->u.<xsl:value-of select="@name"/>, &amp;src->u.<xsl:value-of select="@name"/>);
		break;
		</xsl:when>
		<!-- a compound type, use TYPE_dup() to copy the values -->
		<xsl:otherwise>
	case <xsl:value-of select="$TYPE"/>_<xsl:value-of select="@name"/>:
		<xsl:value-of select="@type"/>_dup(&amp;dst->u.<xsl:value-of select="@name"/>, &amp;src->u.<xsl:value-of select="@name"/>);
		break;
		</xsl:otherwise>
	</xsl:choose>
</xsl:template>

<!-- SEQUENCE types -->
<xsl:template match="xsd:complexType/xsd:all | xsd:complexType/xsd:sequence">
	/* SEQUENCE */
	<xsl:apply-templates select="xsd:element"/>
</xsl:template>

<xsl:template match="xsd:all/xsd:element | xsd:sequence/xsd:element">
	<!-- see what type we are dealing with -->
	<!-- need to handle the case when @type does not exist, eg original_line_style in LineArtClass -->
	<!-- in this case, the type is a named integer -->
	<xsl:choose>
		<!-- a named integer, a boolean or an integer, just use = to copy the values -->
		<xsl:when test="string(@type)='' or @type='xsd:boolean' or @type='xsd:integer'">
			<!-- is it optional -->
			<xsl:choose>
				<xsl:when test="@minOccurs='0'">
	dst->have_<xsl:value-of select="@name"/> = src->have_<xsl:value-of select="@name"/>;
	if(src->have_<xsl:value-of select="@name"/>)
		dst-><xsl:value-of select="@name"/> = src-><xsl:value-of select="@name"/>;
				</xsl:when>
				<xsl:otherwise>
	/* not OPTIONAL */
	dst-><xsl:value-of select="@name"/> = src-><xsl:value-of select="@name"/>;
				</xsl:otherwise>
			</xsl:choose>
		</xsl:when>
		<!-- hexBinary, use OctetString_dup() to copy the values -->
		<xsl:when test="@type='xsd:hexBinary'">
			<!-- is it optional -->
			<xsl:choose>
				<xsl:when test="@minOccurs='0'">
	dst->have_<xsl:value-of select="@name"/> = src->have_<xsl:value-of select="@name"/>;
	if(src->have_<xsl:value-of select="@name"/>)
		OctetString_dup(&amp;dst-><xsl:value-of select="@name"/>, &amp;src-><xsl:value-of select="@name"/>);
				</xsl:when>
				<xsl:otherwise>
	/* not OPTIONAL */
	OctetString_dup(&amp;dst-><xsl:value-of select="@name"/>, &amp;src-><xsl:value-of select="@name"/>);
				</xsl:otherwise>
			</xsl:choose>
		</xsl:when>
		<!-- a compound type, use TYPE_dup() to copy the values -->
		<xsl:otherwise>
			<!-- is it optional -->
			<xsl:choose>
				<xsl:when test="@minOccurs='0'">
	dst->have_<xsl:value-of select="@name"/> = src->have_<xsl:value-of select="@name"/>;
	if(src->have_<xsl:value-of select="@name"/>)
		<xsl:value-of select="@type"/>_dup(&amp;dst-><xsl:value-of select="@name"/>, &amp;src-><xsl:value-of select="@name"/>);
				</xsl:when>
				<xsl:otherwise>
	/* not OPTIONAL */
	<xsl:value-of select="@type"/>_dup(&amp;dst-><xsl:value-of select="@name"/>, &amp;src-><xsl:value-of select="@name"/>);
				</xsl:otherwise>
			</xsl:choose>
		</xsl:otherwise>
	</xsl:choose>
</xsl:template>

</xsl:stylesheet>

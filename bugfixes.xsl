<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html"/>

<!-- This style sheet formats buglist.xml for inclusion on the omniORB -->
<!-- web pages. The html output assumes that it lives at               -->
<!-- http://www.uk.research.att.com/omniORB/omniORBbug.html.           -->

<xsl:template match="/">
    <xsl:apply-templates/>
</xsl:template>

<xsl:template match="buglist">
  <html>
  <head>
  <title>omniORB bugs</title>
  <link REL="stylesheet" HREF="../styles.css" TYPE="text/css"/>
  </head>
  <body background="images/sidebarbg.gif">

<table border="0" cellspacing="0" cellpadding="0"
       width="100%" bgcolor="#000000">
  <tr>
    <td width="15%"><img src="images/omniORB3logo.gif"/></td>
    <td align="center" bgcolor="#000000"><h1 class="banner">
        <font color="#FFFFFF"><em>omniORB <xsl:value-of select="@version"/> Bug List</em></font></h1>
    </td>

    <td width="15%"><p align="center">
      <a href="http://www.uk.research.att.com/">
        <img src="../images/logos/attlogo6876_dk.gif"
             alt="attlogo6876_dk.gif (2280 bytes)" border="0"/></a></p>
    </td>
  </tr>
</table>

<table border="0" width="100%" cellspacing="0" cellpadding="8">
  <tr>
    <td width="100%" colspan="3">
       <a href="index.html"><img src="images/sideHome.gif"
                                 width="120" height="30"
                                 alt="sideHome.gif (2321 bytes)"
                                 border="0"/></a>

       <a href="omniORBForm.html"><img src="images/sideDownload.gif"
                                       width="120" height="33"
                                       alt="sideDownload.gif (2450 bytes)"
                                       border="0"/></a>

       <a href="doc/index.html"><img src="images/sideDocumentation.gif"
                                     width="120" height="33"
                                     alt="sideDocumentation.gif (2512 bytes)"
                                     border="0"/></a>

       <a href="faq.html"><img src="images/sideFAQ.gif"
                           width="120" height="33"
                           alt="sideFAQ.gif (2344 bytes)" border="0"/></a>

       <a href="omniORBbug.html"><img src="images/sidePatch.gif"
                                      width="120" height="33"
                                      alt="sidePatch.gif (2543 bytes)"
                                      border="0"/></a>

       <a href="http://www.uk.research.att.com/search.html"
              ><img src="images/sideSearch.gif"
                    width="120" height="33"
                    alt="sideSearch.gif (2403 bytes)" border="0"/></a>
     </td>
  </tr>
  <tr>
    <td width="100%" bgcolor="#FFFFFF" colspan="3">
      <table border="0" width="100%" cellspacing="3" cellpadding="2">

        <tr><td>

	  <p>
	  The following bugs in omniORB <xsl:value-of
	  select="@version"/> have been fixed. You can get the fixes
	  in three ways:</p>

          <ul><li>Update from <a href="cvs.html">CVS</a> in the
                  <code>"omni3_develop"</code> branch.</li>

              <li>Apply <a href="ftp://ftp.uk.research.att.com/pub/omniORB/omniORB3/omniORB_302_bugfixes.patch">this
                  patch</a> to the omniORB 3.0.2 distribution.</li>

              <li>Download the latest <a
              href="ftp://ftp.uk.research.att.com/pub/omniORB/omniORB_3_snapshots/">source
              snapshot</a>. (Note that this is generated nightly, so the
              latest bugfixes may not appear until tomorrow.)</li>
          </ul>

          <p>
          The bugs pages for earlier versions can be found here:
          <ul>
              <li><a href="bug302.html">3.0.2</a></li>
              <li><a href="bug301.html">3.0.1</a></li>
              <li><a href="bug300.html">3.0.0</a></li>
              <li><a href="bug280.html">2.8.0</a></li>
	      <li><a href="omniORBbug_old.html">for 2.7.1 and earlier</a></li>
          </ul>

          </p>

          <table align="center" width="95%">
            <tr><td colspan="2"><hr/></td></tr>
            <xsl:apply-templates/>
	  </table>

        </td></tr>

      </table>
    </td>

  </tr>

  <tr>
    <td width="100%" colspan="3"> <hr color="#000080"/>
    <p align="center"><em><small>For comments, feedback, etc, please see
    the '<a href="intouch.html">Keeping in touch</a>' page</small>.<br/>
    <small>Copyright 2000 - AT&amp;T Laboratories
    Cambridge</small></em></p>
    </td>
  </tr>

  </table>

  </body>
  </html>
</xsl:template>

<xsl:template match="nobugs">

    <tr><td>No bugs yet.</td></tr>

</xsl:template>


<xsl:template match="bug">

    <tr><td>Summary:</td>
        <td><b><xsl:value-of select="summary"/></b>
            (bug number <xsl:value-of select="@id"/>)
            <a><xsl:attribute name="name">bug<xsl:value-of select="@id"/>
               </xsl:attribute>
            </a>
        </td>
    </tr>

    <tr><td>Date:      </td><td><xsl:value-of select="@date"/></td></tr>
    <tr><td>Fixed by:  </td><td><xsl:value-of select="@user"/></td></tr>

    <xsl:apply-templates/>

    <tr><td colspan="2"><hr/></td></tr>

</xsl:template>

<xsl:template match="summary">
</xsl:template>

<xsl:template match="reported">

    <tr><td>Reported by:</td><td><xsl:value-of select="."/></td></tr>

</xsl:template>

<xsl:template match="link">

    <tr><td>Link for this bug:</td>
        <td><a href="{@href}"><xsl:value-of select="@href"/></a>
        </td>
    </tr>

</xsl:template>

<xsl:template match="text">

    <tr valign="top"><td>Description:</td>
        <td><xsl:apply-templates/></td>
    </tr>

</xsl:template>

<!-- This last rule copies all tags from input to output, so (x)html -->
<!-- can be embedded in <text> tags. -->

<xsl:template match="*|@*|text()">
      <xsl:copy>
        <xsl:apply-templates select="*|@*|text()"/>
      </xsl:copy>
</xsl:template>

</xsl:stylesheet>


# line 72 "idl.yy"
#include <idl.hh>
#include <idl_extern.hh>

#include <fe_private.hh>

#include <stdio.h>

#ifdef __cplusplus
#ifndef __EXTERN_C__
#define __EXTERN_C__
#endif
extern	"C" int yywrap();
#endif

extern char yytext[];


# line 94 "idl.yy"
typedef union
#ifdef __cplusplus
	YYSTYPE
#endif
 {
  AST_Decl		*dcval;		/* Decl value		*/
  UTL_StrList		*slval;		/* String list		*/
  UTL_NameList		*nlval;		/* Name list		*/
  UTL_ExprList		*elval;		/* Expression list	*/
  UTL_LabelList		*llval;		/* Label list		*/
  UTL_DeclList		*dlval;		/* Declaration list	*/
  FE_InterfaceHeader	*ihval;		/* Interface header	*/
  AST_Expression	*exval;		/* Expression value	*/
  AST_UnionLabel	*ulval;		/* Union label		*/
  AST_Field		*ffval;		/* Field value		*/
  AST_Expression::ExprType etval;	/* Expression type	*/
  AST_Argument::Direction dival;	/* Argument direction	*/
  AST_Operation::Flags	ofval;		/* Operation flags	*/
  FE_Declarator		*deval;		/* Declarator value	*/
  idl_bool		bval;		/* Boolean value	*/
  long			ival;		/* Long value		*/
  double		dval;		/* Double value		*/
  float			fval;		/* Float value		*/
  char			cval;		/* Char value		*/
  
  String		*sval;		/* String value		*/
  char			*strval;	/* char * value		*/
  Identifier		*idval;		/* Identifier		*/
  UTL_IdList		*idlist;	/* Identifier list	*/
} YYSTYPE;
# define IDENTIFIER 257
# define CONST 258
# define MODULE 259
# define INTERFACE 260
# define TYPEDEF 261
# define LONG 262
# define SHORT 263
# define UNSIGNED 264
# define DOUBLE 265
# define FLOAT 266
# define CHAR 267
# define WCHAR 268
# define OCTET 269
# define BOOLEAN 270
# define ANY 271
# define STRUCT 272
# define UNION 273
# define SWITCH 274
# define ENUM 275
# define SEQUENCE 276
# define STRING 277
# define WSTRING 278
# define EXCEPTION 279
# define CASE 280
# define DEFAULT 281
# define READONLY 282
# define ATTRIBUTE 283
# define ONEWAY 284
# define IDEMPOTENT 285
# define VOID 286
# define IN 287
# define OUT 288
# define INOUT 289
# define RAISES 290
# define CONTEXT 291
# define INTEGER_LITERAL 292
# define STRING_LITERAL 293
# define CHARACTER_LITERAL 294
# define FLOATING_PT_LITERAL 295
# define TRUETOK 296
# define FALSETOK 297
# define SCOPE_DELIMITOR 298
# define LEFT_SHIFT 299
# define RIGHT_SHIFT 300
# define PRAGMA_ID 301
# define PRAGMA_VERSION 302
# define PRAGMA_PREFIX 303

#if defined(__STDC__) || defined(__cplusplus)
#include <stdlib.h>
#include <string.h>
#else
#include <malloc.h>
#include <memory.h>
#endif

#if !defined(__WIN32__) && !defined(__nextstep__) && !defined(__VMS)
#include <values.h>
#endif

#ifdef __cplusplus

#ifndef yyerror
	void yyerror(const char *);
#endif

#ifndef yylex
#ifdef __EXTERN_C__
	extern "C" { int yylex(void); }
#else
	int yylex(void);
#endif
#endif
	int yyparse(void);

#endif
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
YYSTYPE yylval;
YYSTYPE yyval;
typedef int yytabelem;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
#if YYMAXDEPTH > 0
int yy_yys[YYMAXDEPTH], *yys = yy_yys;
YYSTYPE yy_yyv[YYMAXDEPTH], *yyv = yy_yyv;
#else	/* user does initial allocation */
int *yys;
YYSTYPE *yyv;
#endif
static int yymaxdepth = YYMAXDEPTH;
# define YYERRCODE 256

# line 2317 "idl.yy"

/* programs */

/*
 * ???
 */
int
yywrap()
{
  return 1;
}

/*
 * Report an error situation discovered in a production
 *
 * This does not do anything since we report all error situations through
 * idl_global->err() operations
 */
void
yyerror(const char *)
{
}
yytabelem yyexca[] ={
-1, 0,
	0, 3,
	-2, 0,
-1, 1,
	0, -1,
	-2, 0,
-1, 3,
	0, 3,
	125, 3,
	-2, 0,
-1, 29,
	123, 35,
	-2, 60,
-1, 145,
	91, 234,
	-2, 143,
-1, 162,
	257, 259,
	262, 259,
	263, 259,
	264, 259,
	265, 259,
	266, 259,
	267, 259,
	268, 259,
	269, 259,
	270, 259,
	271, 259,
	277, 259,
	283, 246,
	286, 259,
	298, 259,
	125, 28,
	-2, 0,
-1, 194,
	125, 3,
	-2, 0,
-1, 235,
	125, 250,
	-2, 0,
-1, 282,
	125, 166,
	-2, 0,
-1, 333,
	41, 262,
	-2, 264,
-1, 372,
	125, 189,
	-2, 0,
	};
# define YYNPROD 297
# define YYLAST 541
yytabelem yyact[]={

    77,   142,   346,   143,   362,   239,    62,   139,   168,   254,
   180,    75,   243,   240,    92,   359,   220,   221,    92,    83,
    93,    94,    81,    82,    84,    85,    87,    86,    88,    25,
    26,   119,    27,    95,    96,    97,   158,   408,   403,   157,
   111,   107,   367,   343,    59,   141,   112,   113,   114,    64,
   106,   104,   117,   109,   110,    78,    92,   182,   252,    78,
    72,    83,    93,    94,    81,    82,    84,    85,    87,    86,
    88,    25,    26,    71,    27,    95,    96,    97,   160,     9,
    70,    17,    21,    30,    13,   348,   349,   350,    68,   167,
    67,    16,   364,   363,    16,    25,    26,    78,    27,   121,
   101,   152,    18,   121,    92,    92,   122,   333,   361,   314,
    93,    94,   244,   102,    84,    85,    87,    86,   318,   145,
   100,   148,    27,   108,    22,    23,    24,   140,    99,   214,
    92,   154,   364,   363,   235,    83,    93,    94,    81,    82,
    84,    85,    87,    86,    88,    78,   171,    92,   166,    95,
    96,    97,    83,    93,    94,    81,    82,    84,    85,    87,
    86,    88,   127,   128,   278,    60,     2,    96,   175,   172,
    31,    78,    92,     6,   170,   174,   255,    83,    93,    94,
    81,    82,    84,    85,    87,    86,    88,   173,    78,   382,
   182,   230,    96,   177,   315,   178,   304,   303,   302,     5,
     4,   245,    64,   217,   340,   161,    64,    66,    15,   159,
   156,    15,   155,    78,   256,    92,   115,   218,   258,   257,
    83,    93,    94,    81,    82,    84,    85,    87,    86,   325,
   215,   276,   176,   275,   123,    96,    97,   263,    65,    14,
   189,   190,    14,   234,   153,   126,    64,   125,   399,   205,
   386,    17,   124,    16,    13,   326,    78,   306,   293,   292,
   291,   145,   297,   290,   289,    25,    26,   288,    27,    57,
   296,    56,    18,    55,    92,   208,   179,   209,   210,    54,
    53,    52,   401,   388,   145,    16,   301,    50,   222,   226,
   223,   406,   369,    64,   224,   341,   313,   316,   212,   225,
   211,   256,   404,   398,   317,   258,   257,   355,   344,   183,
   184,   185,   186,   187,   188,    78,   327,   274,   390,   368,
   241,   219,    44,   334,   331,   133,    42,   330,   134,   305,
    43,    69,    76,   407,   397,   328,   202,   378,   379,   354,
   381,   357,   380,   356,   345,   339,   310,   338,   353,   337,
   332,   324,   299,   331,   295,   279,   365,   233,   193,   311,
   238,   236,   201,   200,   256,   265,   309,   370,   258,   257,
    15,   103,   105,   282,   308,   131,   312,    40,   323,   385,
   387,   294,   331,   392,    64,   393,   389,   391,   383,   266,
   140,   298,   264,   271,   272,   273,   145,   400,   269,   270,
   319,    14,    15,   262,   165,   232,   151,    92,   267,   268,
   227,   228,   229,    91,   231,   150,    90,   191,   129,   216,
   149,   322,   287,   286,   242,   197,   137,    47,   395,   396,
   377,   376,   374,    14,   394,   373,   372,   371,   358,   351,
   335,   321,   183,   184,   185,   186,   187,   188,    78,   285,
   196,   136,    46,   284,   320,   283,   281,   237,   195,   135,
    45,   352,   260,   118,    58,    38,   277,   192,   130,    39,
   147,   120,   259,   251,   250,   204,   249,   203,   248,   247,
   246,   199,   116,    51,   198,   162,   138,    48,    20,    19,
   280,   194,   132,    41,    12,    11,    10,    37,    36,     8,
    35,     7,    34,    33,    32,     3,     1,    29,   206,   144,
   207,   347,    80,    79,    73,    98,   384,   181,   169,   300,
    28,   336,   329,   164,   375,   360,   261,   213,   342,    49,
   163,   405,   402,   366,    89,   253,   146,   307,    74,    63,
    61 };
yytabelem yypact[]={

  -177,-10000000,-10000000,  -177,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,   229,
-10000000,-10000000,   222,   221,   220,   214,   212,   210,  -201,   -42,
  -152,  -134,  -239,  -239,  -253,  -152,  -152,  -152,    93,-10000000,
-10000000,  -152,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,  -267,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,  -159,-10000000,-10000000,-10000000,-10000000,-10000000,   190,
   187,   185,-10000000,-10000000,  -100,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,  -267,-10000000,-10000000,  -267,
  -267,-10000000,-10000000,-10000000,-10000000,-10000000,  -239,-10000000,  -152,-10000000,
  -152,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,  -161,-10000000,   184,
  -152,    89,    87,  -254,  -259,    86,  -196,    82,-10000000,-10000000,
  -267,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,  -152,-10000000,   150,
   150,   150,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,    -7,   256,   254,   139,-10000000,-10000000,-10000000,    79,
   123,   283,  -283,   245,   252,-10000000,-10000000,    17,    17,    17,
  -267,-10000000,   150,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,  -127,   182,-10000000,  -177,  -243,   280,  -145,    76,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,  -225,  -110,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,   175,   150,   150,   150,
   150,   150,   150,   150,   150,   150,   150,-10000000,-10000000,-10000000,
   276,   171,   169,-10000000,-10000000,  -243,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,   208,   205,   204,   201,
   200,   199,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,  -267,  -239,
  -152,   139,   150,-10000000,   123,   283,  -283,   245,   245,   252,
   252,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,   150,-10000000,    73,
    72,    71,  -243,  -152,   198,  -153,    69,   253,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,   -85,  -139,  -267,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,  -267,  -163,-10000000,-10000000,-10000000,-10000000,   136,
   196,   275,  -145,  -152,    67,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,    81,   251,  -247,   267,  -202,
-10000000,-10000000,-10000000,-10000000,-10000000,   266,-10000000,-10000000,-10000000,-10000000,
-10000000,  -148,  -152,  -249,   279,-10000000,   248,   -85,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,    64,  -148,  -201,   191,  -188,   225,   150,   278,  -239,
  -202,  -152,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,   262,-10000000,-10000000,   189,  -152,   224,  -255,-10000000,-10000000,
-10000000,-10000000,   261,-10000000,-10000000,   247,-10000000,  -256,-10000000 };
yytabelem yypgo[]={

     0,     5,   165,   540,   539,   538,    11,   238,    90,   537,
   207,   536,   535,   534,   332,     9,    10,   533,   532,   531,
     7,   530,   529,   528,   527,   526,   525,   524,    45,   523,
   522,   521,   520,   519,     8,   518,   174,   146,   169,   187,
   175,   168,   232,   517,    89,   129,     4,   516,   515,    88,
    80,    73,   331,   514,   513,   512,     6,    60,   511,   510,
     1,     3,   509,   508,   507,     0,   506,   166,   505,   200,
   504,   199,   503,   173,   502,   501,   500,   499,   498,   497,
   496,   495,   494,   493,   492,   491,   490,   489,   488,   487,
   486,   485,   484,   483,   482,   481,   480,   479,   478,   477,
   476,   475,   474,   473,   472,   471,   470,   469,   468,   467,
   466,   465,   464,   463,   462,   461,   460,   459,   458,   457,
   456,   164,   134,   455,   454,   453,   452,   451,   450,   449,
   441,   440,   439,   438,   437,    15,   436,   435,   434,   432,
   431,   430,   429,   428,   427,   426,   425,   424,   423,    12,
   422,   421,   420,   419,   418,   417,   416,   415,   414,   413,
   406,   405,   404,   403,   400,   381,   378,   377,   375,   358,
   355,   354,   351,   350,   349,   348,   347,   345,   344,     2,
   343,   342,   341,   340,   339,   338,   337,   334,   333,   330,
   328,   326,   325,   322 };
yytabelem yyr1[]={

     0,    66,    67,    67,    70,    68,    72,    68,    74,    68,
    76,    68,    78,    68,    79,    68,    68,    68,    68,    83,
    84,    85,    86,    77,    75,    75,    89,    90,    92,    87,
    93,    64,    32,    94,    22,    22,    91,    91,    96,    95,
    97,    95,    98,    95,   100,    95,   102,    95,   103,    95,
    20,   104,    21,    21,    16,   105,    16,   106,    16,    65,
    88,   107,   108,   109,   110,    71,    48,    48,    48,    48,
    48,    48,    48,    48,    33,    34,    35,    35,    36,    36,
    37,    37,    38,    38,    38,    39,    39,    39,    40,    40,
    40,    40,    41,    41,    41,    41,    42,    42,    42,    43,
    43,    43,    43,    43,    43,    44,   111,    69,    69,    69,
    69,   113,   112,     1,     1,     2,     2,     2,    56,    56,
    56,    56,    56,    56,     4,     4,     4,     3,     3,     3,
    15,    15,    15,    28,   114,    29,    29,    60,    60,    30,
   115,    31,    31,    61,    62,    49,    49,    54,    54,    54,
    55,    55,    55,    52,    52,    52,    50,    50,    57,    51,
    53,   116,   117,   118,   120,     7,   119,   122,   122,   123,
   124,   121,   125,   121,   126,   127,   128,   129,   130,   131,
   132,   134,    10,     9,     9,     9,     9,     9,     9,   133,
   136,   136,   137,   138,   135,   139,   135,    26,    27,    27,
   140,    46,   141,   142,    46,   143,    47,   144,   145,   146,
   148,     8,   147,   151,   150,   150,   149,   152,   153,     5,
     5,   154,   155,    13,   157,   158,     6,     6,   156,   160,
   161,    14,    14,   159,   162,    11,    24,    25,    25,   163,
   164,    45,   165,   166,    99,    63,    63,   167,   168,   169,
   170,    73,   171,   172,   174,   175,   101,    59,    59,    59,
    12,    12,   176,   173,   177,   173,   173,   178,   181,   180,
   180,   182,   183,   179,    58,    58,    58,   184,   185,    23,
    23,   186,   187,    17,    17,    18,   188,    19,    19,   189,
   190,    81,   191,   192,    80,   193,    82 };
yytabelem yyr2[]={

     0,     2,     4,     0,     1,     7,     1,     7,     1,     7,
     1,     7,     1,     7,     1,     7,     2,     2,     2,     1,
     1,     1,     1,    19,     2,     2,     1,     1,     1,    15,
     1,     7,     5,     1,     7,     1,     4,     0,     1,     7,
     1,     7,     1,     7,     1,     7,     1,     7,     1,     7,
     5,     1,     9,     1,     3,     1,     7,     1,     9,     3,
     3,     1,     1,     1,     1,    19,     2,     2,     2,     2,
     2,     3,     3,     3,     2,     2,     2,     7,     2,     7,
     2,     7,     2,     7,     7,     2,     7,     7,     2,     7,
     7,     7,     2,     5,     5,     5,     3,     2,     7,     3,
     3,     3,     3,     3,     3,     3,     1,     6,     2,     2,
     2,     1,     7,     2,     2,     3,     2,     3,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     3,     2,     3,     5,     1,     9,     1,     2,     2,     5,
     1,     9,     1,     3,     3,     2,     2,     3,     5,     3,
     5,     7,     5,     3,     3,     5,     3,     3,     3,     3,
     3,     1,     1,     1,     1,    19,     4,     4,     0,     1,
     1,    11,     1,     7,     1,     1,     1,     1,     1,     1,
     1,     1,    35,     3,     3,     3,     3,     2,     3,     4,
     4,     0,     1,     1,    11,     1,     7,     5,     5,     1,
     1,     7,     1,     1,    11,     1,     7,     1,     1,     1,
     1,    19,     4,     1,     8,     0,     3,     1,     1,    13,
     5,     1,     1,    11,     1,     1,    13,     3,     3,     1,
     1,    13,     3,     3,     1,     7,     5,     5,     1,     1,
     1,    11,     1,     1,    13,     3,     1,     1,     1,     1,
     1,    19,     1,     1,     1,     1,    21,     3,     3,     1,
     2,     3,     1,     7,     1,     9,     3,     4,     1,     8,
     0,     1,     1,    11,     3,     3,     3,     1,     1,    13,
     1,     1,     1,    13,     1,     5,     1,     9,     1,     1,
     1,    11,     1,     1,    11,     1,     7 };
yytabelem yychk[]={

-10000000,   -66,   -67,   -68,   -69,   -71,   -73,   -75,   -77,   256,
   -80,   -81,   -82,   261,    -7,   -10,    -8,   258,   279,   -87,
   -88,   259,   301,   302,   303,   272,   273,   275,   -32,   -64,
   260,   -67,   -70,   -72,   -74,   -76,   -78,   -79,  -111,  -107,
  -167,   -83,  -191,  -189,  -193,  -116,  -126,  -144,   -89,   -22,
    58,   -93,    59,    59,    59,    59,    59,    59,  -112,    -1,
    -2,    -3,   -56,    -4,   -16,    -7,   -10,    -8,   -49,   -52,
   -50,   -51,   -57,   -53,    -5,    -6,   -14,   -65,   298,   -54,
   -55,   265,   266,   262,   267,   268,   270,   269,   271,   -13,
  -156,  -159,   257,   263,   264,   276,   277,   278,   -48,   -49,
   -50,   -57,   -51,   -52,    -6,   -14,   -16,   -65,   257,   -16,
   -16,   293,   -65,   -65,   -65,   123,   -94,   -65,  -113,   298,
  -105,   262,   265,    44,    62,    60,    60,   262,   263,  -154,
  -108,  -168,   -84,  -192,  -190,  -117,  -127,  -145,   -90,   -20,
   -16,   -28,   -60,   -61,   -62,   -65,   -11,  -106,   -65,  -152,
  -157,  -160,   262,    60,   -65,   123,   123,   293,   295,   123,
   274,   123,   -91,   -21,   -29,  -162,   -65,   -44,   -34,   -35,
   -36,   -37,   -38,   -39,   -40,   -41,   -42,    43,    45,   126,
   -16,   -43,    40,   292,   293,   294,   295,   296,   297,   -44,
   -44,  -155,  -109,  -169,   -85,  -118,  -128,  -146,   -92,   -95,
   -69,   -71,   -73,   -99,  -101,   256,   -63,   -59,   282,   284,
   285,    44,    44,   -24,   -45,    91,  -153,   124,    94,    38,
   299,   300,    43,    45,    42,    47,    37,   -42,   -42,   -42,
   -34,  -158,  -161,    -2,    61,  -122,   -67,  -119,  -121,    -1,
   256,    40,  -147,  -149,   257,   125,   -96,   -97,   -98,  -100,
  -102,  -103,   283,   -12,   -15,   286,   -56,    -6,   -16,  -104,
  -114,   -25,  -163,    62,   -36,   -37,   -38,   -39,   -39,   -40,
   -40,   -41,   -41,   -41,    41,    62,    62,  -110,  -121,  -170,
   -86,  -120,  -122,  -123,  -125,  -129,  -148,  -150,    59,    59,
    59,    59,    59,    59,  -165,  -171,   -16,   -60,   -45,   -44,
   -33,   -34,   125,   125,   125,   -28,    59,    -9,   -49,   -50,
   -57,   -51,    -8,   -16,   262,   125,    44,   -15,   257,  -164,
  -124,  -130,  -151,  -166,  -172,    93,    59,    41,  -149,   -30,
   -61,   -65,  -173,    40,   256,  -131,   -31,  -174,  -176,  -177,
   123,    44,   -23,   290,    41,  -178,  -179,   -58,   287,   288,
   289,  -132,  -115,  -175,  -184,    41,  -180,  -182,  -133,  -135,
   -26,   256,   -46,   281,   280,   -61,   -17,   291,    40,    44,
   -15,  -134,  -136,  -137,  -139,   -27,  -140,  -141,  -186,  -185,
  -181,  -183,   125,  -135,   -47,    -1,    59,   -46,    58,   -34,
    40,   -20,  -179,   -61,  -138,  -143,  -142,  -187,    41,    59,
   -60,    58,   -18,   293,    41,   -19,    44,  -188,   293 };
yytabelem yydef[]={

    -2,    -2,     1,    -2,     4,     6,     8,    10,    12,    14,
    16,    17,    18,   106,   108,   109,   110,    61,   247,    24,
    25,    19,   292,   289,   295,   161,   174,   207,    26,    -2,
    30,     2,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    32,
    33,     0,     5,     7,     9,    11,    13,    15,   107,   111,
   113,   114,   115,   116,   117,   127,   128,   129,   118,   119,
   120,   121,   122,   123,   124,   125,   126,    54,    55,   145,
   146,   153,   154,   147,   156,   157,   159,   158,   160,     0,
   227,   232,    59,   149,     0,   221,   228,   233,    62,    66,
    67,    68,    69,    70,    71,    72,    73,   248,    20,   293,
   290,   296,   162,   175,   208,    27,     0,    31,     0,    57,
     0,   148,   155,   217,   220,   224,   229,   150,   152,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    37,    34,
    53,   112,   136,   137,   138,    -2,   144,     0,    56,     0,
     0,     0,   151,   222,    63,   249,    21,   294,   291,   163,
   176,   209,    -2,    50,   133,     0,    58,   218,   105,    75,
    76,    78,    80,    82,    85,    88,    92,     0,     0,     0,
    96,    97,     0,    99,   100,   101,   102,   103,   104,   225,
   230,     0,     0,   168,    -2,     0,     0,     0,     0,    36,
    38,    40,    42,    44,    46,    48,     0,     0,   245,   257,
   258,    51,   134,   235,   238,   239,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    93,    94,    95,
     0,     0,     0,   223,    64,    -2,    22,   164,   168,   169,
   172,   177,   210,   215,   216,    29,     0,     0,     0,     0,
     0,     0,   242,   252,   260,   261,   130,   131,   132,     0,
     0,   236,     0,   219,    77,    79,    81,    83,    84,    86,
    87,    89,    90,    91,    98,   226,   231,     0,   167,     0,
     0,     0,    -2,     0,     0,     0,     0,   212,    39,    41,
    43,    45,    47,    49,     0,     0,    52,   135,   237,   240,
    65,    74,   251,    23,   165,   170,   173,   178,   183,   184,
   185,   186,   187,   188,   147,   211,   213,   243,   253,     0,
     0,     0,     0,     0,     0,   241,   171,   179,   214,   244,
   142,   143,   254,    -2,   266,     0,   139,   280,     0,     0,
   180,   140,   255,   277,   263,     0,   270,   271,   274,   275,
   276,     0,     0,   284,     0,   265,   267,     0,   181,   191,
   192,   195,   199,   200,   202,   141,   256,   281,   278,   268,
   272,     0,    -2,     0,     0,   197,     0,     0,     0,     0,
     0,     0,   182,   190,   193,   205,   196,   198,   201,   203,
   282,     0,   269,   273,     0,     0,     0,     0,   279,   194,
   206,   204,     0,   288,   283,   285,   286,     0,   287 };
typedef struct
#ifdef __cplusplus
	yytoktype
#endif
{ char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"IDENTIFIER",	257,
	"CONST",	258,
	"MODULE",	259,
	"INTERFACE",	260,
	"TYPEDEF",	261,
	"LONG",	262,
	"SHORT",	263,
	"UNSIGNED",	264,
	"DOUBLE",	265,
	"FLOAT",	266,
	"CHAR",	267,
	"WCHAR",	268,
	"OCTET",	269,
	"BOOLEAN",	270,
	"ANY",	271,
	"STRUCT",	272,
	"UNION",	273,
	"SWITCH",	274,
	"ENUM",	275,
	"SEQUENCE",	276,
	"STRING",	277,
	"WSTRING",	278,
	"EXCEPTION",	279,
	"CASE",	280,
	"DEFAULT",	281,
	"READONLY",	282,
	"ATTRIBUTE",	283,
	"ONEWAY",	284,
	"IDEMPOTENT",	285,
	"VOID",	286,
	"IN",	287,
	"OUT",	288,
	"INOUT",	289,
	"RAISES",	290,
	"CONTEXT",	291,
	"INTEGER_LITERAL",	292,
	"STRING_LITERAL",	293,
	"CHARACTER_LITERAL",	294,
	"FLOATING_PT_LITERAL",	295,
	"TRUETOK",	296,
	"FALSETOK",	297,
	"SCOPE_DELIMITOR",	298,
	"LEFT_SHIFT",	299,
	"RIGHT_SHIFT",	300,
	"PRAGMA_ID",	301,
	"PRAGMA_VERSION",	302,
	"PRAGMA_PREFIX",	303,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"start : definitions",
	"definitions : definition definitions",
	"definitions : /* empty */",
	"definition : type_dcl",
	"definition : type_dcl ';'",
	"definition : const_dcl",
	"definition : const_dcl ';'",
	"definition : exception",
	"definition : exception ';'",
	"definition : interface_def",
	"definition : interface_def ';'",
	"definition : module",
	"definition : module ';'",
	"definition : error",
	"definition : error ';'",
	"definition : pragma_id",
	"definition : pragma_version",
	"definition : pragma_prefix",
	"module : MODULE",
	"module : MODULE IDENTIFIER",
	"module : MODULE IDENTIFIER '{'",
	"module : MODULE IDENTIFIER '{' definitions",
	"module : MODULE IDENTIFIER '{' definitions '}'",
	"interface_def : interface",
	"interface_def : forward",
	"interface : interface_header",
	"interface : interface_header '{'",
	"interface : interface_header '{' exports",
	"interface : interface_header '{' exports '}'",
	"interface_decl : INTERFACE",
	"interface_decl : INTERFACE id",
	"interface_header : interface_decl inheritance_spec",
	"inheritance_spec : ':'",
	"inheritance_spec : ':' at_least_one_scoped_name",
	"inheritance_spec : /* empty */",
	"exports : exports export",
	"exports : /* empty */",
	"export : type_dcl",
	"export : type_dcl ';'",
	"export : const_dcl",
	"export : const_dcl ';'",
	"export : exception",
	"export : exception ';'",
	"export : attribute",
	"export : attribute ';'",
	"export : operation",
	"export : operation ';'",
	"export : error",
	"export : error ';'",
	"at_least_one_scoped_name : scoped_name scoped_names",
	"scoped_names : scoped_names ','",
	"scoped_names : scoped_names ',' scoped_name",
	"scoped_names : /* empty */",
	"scoped_name : id",
	"scoped_name : SCOPE_DELIMITOR",
	"scoped_name : SCOPE_DELIMITOR id",
	"scoped_name : scoped_name SCOPE_DELIMITOR",
	"scoped_name : scoped_name SCOPE_DELIMITOR id",
	"id : IDENTIFIER",
	"forward : interface_decl",
	"const_dcl : CONST",
	"const_dcl : CONST const_type",
	"const_dcl : CONST const_type id",
	"const_dcl : CONST const_type id '='",
	"const_dcl : CONST const_type id '=' expression",
	"const_type : integer_type",
	"const_type : char_type",
	"const_type : octet_type",
	"const_type : boolean_type",
	"const_type : floating_pt_type",
	"const_type : string_type_spec",
	"const_type : wstring_type_spec",
	"const_type : scoped_name",
	"expression : const_expr",
	"const_expr : or_expr",
	"or_expr : xor_expr",
	"or_expr : or_expr '|' xor_expr",
	"xor_expr : and_expr",
	"xor_expr : xor_expr '^' and_expr",
	"and_expr : shift_expr",
	"and_expr : and_expr '&' shift_expr",
	"shift_expr : add_expr",
	"shift_expr : shift_expr LEFT_SHIFT add_expr",
	"shift_expr : shift_expr RIGHT_SHIFT add_expr",
	"add_expr : mult_expr",
	"add_expr : add_expr '+' mult_expr",
	"add_expr : add_expr '-' mult_expr",
	"mult_expr : unary_expr",
	"mult_expr : mult_expr '*' unary_expr",
	"mult_expr : mult_expr '/' unary_expr",
	"mult_expr : mult_expr '%' unary_expr",
	"unary_expr : primary_expr",
	"unary_expr : '+' primary_expr",
	"unary_expr : '-' primary_expr",
	"unary_expr : '~' primary_expr",
	"primary_expr : scoped_name",
	"primary_expr : literal",
	"primary_expr : '(' const_expr ')'",
	"literal : INTEGER_LITERAL",
	"literal : STRING_LITERAL",
	"literal : CHARACTER_LITERAL",
	"literal : FLOATING_PT_LITERAL",
	"literal : TRUETOK",
	"literal : FALSETOK",
	"positive_int_expr : const_expr",
	"type_dcl : TYPEDEF",
	"type_dcl : TYPEDEF type_declarator",
	"type_dcl : struct_type",
	"type_dcl : union_type",
	"type_dcl : enum_type",
	"type_declarator : type_spec",
	"type_declarator : type_spec at_least_one_declarator",
	"type_spec : simple_type_spec",
	"type_spec : constructed_type_spec",
	"simple_type_spec : base_type_spec",
	"simple_type_spec : template_type_spec",
	"simple_type_spec : scoped_name",
	"base_type_spec : integer_type",
	"base_type_spec : floating_pt_type",
	"base_type_spec : char_type",
	"base_type_spec : boolean_type",
	"base_type_spec : octet_type",
	"base_type_spec : any_type",
	"template_type_spec : sequence_type_spec",
	"template_type_spec : string_type_spec",
	"template_type_spec : wstring_type_spec",
	"constructed_type_spec : struct_type",
	"constructed_type_spec : union_type",
	"constructed_type_spec : enum_type",
	"param_type_spec : base_type_spec",
	"param_type_spec : string_type_spec",
	"param_type_spec : scoped_name",
	"at_least_one_declarator : declarator declarators",
	"declarators : declarators ','",
	"declarators : declarators ',' declarator",
	"declarators : /* empty */",
	"declarator : simple_declarator",
	"declarator : complex_declarator",
	"at_least_one_simple_declarator : simple_declarator simple_declarators",
	"simple_declarators : simple_declarators ','",
	"simple_declarators : simple_declarators ',' simple_declarator",
	"simple_declarators : /* empty */",
	"simple_declarator : id",
	"complex_declarator : array_declarator",
	"integer_type : signed_int",
	"integer_type : unsigned_int",
	"signed_int : LONG",
	"signed_int : LONG LONG",
	"signed_int : SHORT",
	"unsigned_int : UNSIGNED LONG",
	"unsigned_int : UNSIGNED LONG LONG",
	"unsigned_int : UNSIGNED SHORT",
	"floating_pt_type : DOUBLE",
	"floating_pt_type : FLOAT",
	"floating_pt_type : LONG DOUBLE",
	"char_type : CHAR",
	"char_type : WCHAR",
	"octet_type : OCTET",
	"boolean_type : BOOLEAN",
	"any_type : ANY",
	"struct_type : STRUCT",
	"struct_type : STRUCT id",
	"struct_type : STRUCT id '{'",
	"struct_type : STRUCT id '{' at_least_one_member",
	"struct_type : STRUCT id '{' at_least_one_member '}'",
	"at_least_one_member : member members",
	"members : members member",
	"members : /* empty */",
	"member : type_spec",
	"member : type_spec at_least_one_declarator",
	"member : type_spec at_least_one_declarator ';'",
	"member : error",
	"member : error ';'",
	"union_type : UNION",
	"union_type : UNION id",
	"union_type : UNION id SWITCH",
	"union_type : UNION id SWITCH '('",
	"union_type : UNION id SWITCH '(' switch_type_spec",
	"union_type : UNION id SWITCH '(' switch_type_spec ')'",
	"union_type : UNION id SWITCH '(' switch_type_spec ')' '{'",
	"union_type : UNION id SWITCH '(' switch_type_spec ')' '{' at_least_one_case_branch",
	"union_type : UNION id SWITCH '(' switch_type_spec ')' '{' at_least_one_case_branch '}'",
	"switch_type_spec : integer_type",
	"switch_type_spec : char_type",
	"switch_type_spec : octet_type",
	"switch_type_spec : boolean_type",
	"switch_type_spec : enum_type",
	"switch_type_spec : scoped_name",
	"at_least_one_case_branch : case_branch case_branches",
	"case_branches : case_branches case_branch",
	"case_branches : /* empty */",
	"case_branch : at_least_one_case_label",
	"case_branch : at_least_one_case_label element_spec",
	"case_branch : at_least_one_case_label element_spec ';'",
	"case_branch : error",
	"case_branch : error ';'",
	"at_least_one_case_label : case_label case_labels",
	"case_labels : case_labels case_label",
	"case_labels : /* empty */",
	"case_label : DEFAULT",
	"case_label : DEFAULT ':'",
	"case_label : CASE",
	"case_label : CASE const_expr",
	"case_label : CASE const_expr ':'",
	"element_spec : type_spec",
	"element_spec : type_spec declarator",
	"enum_type : ENUM",
	"enum_type : ENUM id",
	"enum_type : ENUM id '{'",
	"enum_type : ENUM id '{' at_least_one_enumerator",
	"enum_type : ENUM id '{' at_least_one_enumerator '}'",
	"at_least_one_enumerator : enumerator enumerators",
	"enumerators : enumerators ','",
	"enumerators : enumerators ',' enumerator",
	"enumerators : /* empty */",
	"enumerator : IDENTIFIER",
	"sequence_type_spec : seq_head ','",
	"sequence_type_spec : seq_head ',' positive_int_expr",
	"sequence_type_spec : seq_head ',' positive_int_expr '>'",
	"sequence_type_spec : seq_head '>'",
	"seq_head : SEQUENCE",
	"seq_head : SEQUENCE '<'",
	"seq_head : SEQUENCE '<' simple_type_spec",
	"string_type_spec : string_head '<'",
	"string_type_spec : string_head '<' positive_int_expr",
	"string_type_spec : string_head '<' positive_int_expr '>'",
	"string_type_spec : string_head",
	"string_head : STRING",
	"wstring_type_spec : wstring_head '<'",
	"wstring_type_spec : wstring_head '<' positive_int_expr",
	"wstring_type_spec : wstring_head '<' positive_int_expr '>'",
	"wstring_type_spec : wstring_head",
	"wstring_head : WSTRING",
	"array_declarator : id",
	"array_declarator : id at_least_one_array_dim",
	"at_least_one_array_dim : array_dim array_dims",
	"array_dims : array_dims array_dim",
	"array_dims : /* empty */",
	"array_dim : '['",
	"array_dim : '[' positive_int_expr",
	"array_dim : '[' positive_int_expr ']'",
	"attribute : opt_readonly ATTRIBUTE",
	"attribute : opt_readonly ATTRIBUTE param_type_spec",
	"attribute : opt_readonly ATTRIBUTE param_type_spec at_least_one_simple_declarator",
	"opt_readonly : READONLY",
	"opt_readonly : /* empty */",
	"exception : EXCEPTION",
	"exception : EXCEPTION id",
	"exception : EXCEPTION id '{'",
	"exception : EXCEPTION id '{' members",
	"exception : EXCEPTION id '{' members '}'",
	"operation : opt_op_attribute op_type_spec",
	"operation : opt_op_attribute op_type_spec IDENTIFIER",
	"operation : opt_op_attribute op_type_spec IDENTIFIER parameter_list",
	"operation : opt_op_attribute op_type_spec IDENTIFIER parameter_list opt_raises",
	"operation : opt_op_attribute op_type_spec IDENTIFIER parameter_list opt_raises opt_context",
	"opt_op_attribute : ONEWAY",
	"opt_op_attribute : IDEMPOTENT",
	"opt_op_attribute : /* empty */",
	"op_type_spec : param_type_spec",
	"op_type_spec : VOID",
	"parameter_list : '('",
	"parameter_list : '(' ')'",
	"parameter_list : '('",
	"parameter_list : '(' at_least_one_parameter ')'",
	"parameter_list : error",
	"at_least_one_parameter : parameter parameters",
	"parameters : parameters ','",
	"parameters : parameters ',' parameter",
	"parameters : /* empty */",
	"parameter : direction",
	"parameter : direction param_type_spec",
	"parameter : direction param_type_spec simple_declarator",
	"direction : IN",
	"direction : OUT",
	"direction : INOUT",
	"opt_raises : RAISES",
	"opt_raises : RAISES '('",
	"opt_raises : RAISES '(' at_least_one_scoped_name ')'",
	"opt_raises : /* empty */",
	"opt_context : CONTEXT",
	"opt_context : CONTEXT '('",
	"opt_context : CONTEXT '(' at_least_one_string_literal ')'",
	"opt_context : /* empty */",
	"at_least_one_string_literal : STRING_LITERAL string_literals",
	"string_literals : string_literals ','",
	"string_literals : string_literals ',' STRING_LITERAL",
	"string_literals : /* empty */",
	"pragma_version : PRAGMA_VERSION",
	"pragma_version : PRAGMA_VERSION scoped_name",
	"pragma_version : PRAGMA_VERSION scoped_name FLOATING_PT_LITERAL",
	"pragma_id : PRAGMA_ID",
	"pragma_id : PRAGMA_ID scoped_name",
	"pragma_id : PRAGMA_ID scoped_name STRING_LITERAL",
	"pragma_prefix : PRAGMA_PREFIX",
	"pragma_prefix : PRAGMA_PREFIX STRING_LITERAL",
};
#endif /* YYDEBUG */
# line	1 "/usr/ccs/bin/yaccpar"
/*
 * Copyright (c) 1993 by Sun Microsystems, Inc.
 */


/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	return(0)
#define YYABORT		return(1)
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#define YYNEW(type)	malloc(sizeof(type) * yynewmax)
#define YYCOPY(to, from, type) \
	(type *) memcpy(to, (char *) from, yynewmax * sizeof(type))
#define YYENLARGE( from, type) \
	(type *) realloc((char *) from, yynewmax * sizeof(type))
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-10000000)

/*
** global variables used by the parser
*/
YYSTYPE *yypv;			/* top of value stack */
int *yyps;			/* top of state stack */

int yystate;			/* current state */
int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */
int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */



#ifdef YYNMBCHARS
#define YYLEX()		yycvtok(yylex())
/*
** yycvtok - return a token if i is a wchar_t value that exceeds 255.
**	If i<255, i itself is the token.  If i>255 but the neither 
**	of the 30th or 31st bit is on, i is already a token.
*/
#if defined(__STDC__) || defined(__cplusplus)
int yycvtok(int i)
#else
int yycvtok(i) int i;
#endif
{
	int first = 0;
	int last = YYNMBCHARS - 1;
	int mid;
	wchar_t j;

	if(i&0x60000000){/*Must convert to a token. */
		if( yymbchars[last].character < i ){
			return i;/*Giving up*/
		}
		while ((last>=first)&&(first>=0)) {/*Binary search loop*/
			mid = (first+last)/2;
			j = yymbchars[mid].character;
			if( j==i ){/*Found*/ 
				return yymbchars[mid].tvalue;
			}else if( j<i ){
				first = mid + 1;
			}else{
				last = mid -1;
			}
		}
		/*No entry in the table.*/
		return i;/* Giving up.*/
	}else{/* i is already a token. */
		return i;
	}
}
#else/*!YYNMBCHARS*/
#define YYLEX()		yylex()
#endif/*!YYNMBCHARS*/

/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
#if defined(__STDC__) || defined(__cplusplus)
int yyparse(void)
#else
int yyparse()
#endif
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */

#if defined(__cplusplus) || defined(lint)
/*
	hacks to please C++ and lint - goto's inside switch should never be
	executed; yypvt is set to 0 to avoid "used before set" warning.
*/
	static int __yaccpar_lint_hack__ = 0;
	switch (__yaccpar_lint_hack__)
	{
		case 1: goto yyerrlab;
		case 2: goto yynewstate;
	}
	yypvt = 0;
#endif

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

#if YYMAXDEPTH <= 0
	if (yymaxdepth <= 0)
	{
		if ((yymaxdepth = YYEXPAND(0)) <= 0)
		{
			yyerror("yacc initialization error");
			YYABORT;
		}
	}
#endif

	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */
	goto yystack;	/* moved from 6 lines above to here to please C++ */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ yymaxdepth ] )	/* room on stack? */
		{
			/*
			** reallocate and recover.  Note that pointers
			** have to be reset, or bad things will happen
			*/
			int yyps_index = (yy_ps - yys);
			int yypv_index = (yy_pv - yyv);
			int yypvt_index = (yypvt - yyv);
			int yynewmax;
#ifdef YYEXPAND
			yynewmax = YYEXPAND(yymaxdepth);
#else
			yynewmax = 2 * yymaxdepth;	/* double table size */
			if (yymaxdepth == YYMAXDEPTH)	/* first time growth */
			{
				char *newyys = (char *)YYNEW(int);
				char *newyyv = (char *)YYNEW(YYSTYPE);
				if (newyys != 0 && newyyv != 0)
				{
					yys = YYCOPY(newyys, yys, int);
					yyv = YYCOPY(newyyv, yyv, YYSTYPE);
				}
				else
					yynewmax = 0;	/* failed */
			}
			else				/* not first time */
			{
				yys = YYENLARGE(yys, int);
				yyv = YYENLARGE(yyv, YYSTYPE);
				if (yys == 0 || yyv == 0)
					yynewmax = 0;	/* failed */
			}
#endif
			if (yynewmax <= yymaxdepth)	/* tables not expanded */
			{
				yyerror( "yacc stack overflow" );
				YYABORT;
			}
			yymaxdepth = yynewmax;

			yy_ps = yys + yyps_index;
			yy_pv = yyv + yypv_index;
			yypvt = yyv + yypvt_index;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = YYLEX() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			printf( "Received token " );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = YYLEX() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				printf( "Received token " );
				if ( yychar == 0 )
					printf( "end-of-file\n" );
				else if ( yychar < 0 )
					printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax error" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
			skip_init:
				yynerrs++;
				/* FALLTHRU */
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					printf( "Error recovery discards " );
					if ( yychar == 0 )
						printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 4:
# line 240 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_TypeDeclSeen);
        } break;
case 5:
# line 244 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        } break;
case 6:
# line 248 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_ConstDeclSeen);
        } break;
case 7:
# line 252 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        } break;
case 8:
# line 256 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_ExceptDeclSeen);
        } break;
case 9:
# line 260 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        } break;
case 10:
# line 264 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceDeclSeen);
        } break;
case 11:
# line 268 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        } break;
case 12:
# line 272 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_ModuleDeclSeen);
        } break;
case 13:
# line 276 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        } break;
case 14:
# line 280 "idl.yy"
{
	  idl_global->err()->syntax_error(idl_global->parse_state());
	} break;
case 15:
# line 284 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
	  yyerrok;
	} break;
case 19:
# line 294 "idl.yy"
{
	    idl_global->set_parse_state(IDL_GlobalData::PS_ModuleSeen);
	  } break;
case 20:
# line 298 "idl.yy"
{
	    UTL_ScopedName 	*n =
		new UTL_ScopedName(new Identifier(yypvt[-0].strval, 1, 0, I_FALSE), NULL);
	    AST_Module		*m = NULL;
	    UTL_Scope		*s = idl_global->scopes()->top_non_null();
	    UTL_StrList		*p = idl_global->pragmas();	

	    idl_global->set_parse_state(IDL_GlobalData::PS_ModuleIDSeen);
	    /*
	     * Make a new module and add it to the enclosing scope
	     */
	    if (s != NULL) {
	      m = idl_global->gen()->create_module(n, p);
	      AST_Module* sb = s->fe_add_module(m); if (sb != NULL) m = sb;
	    }
	    /*
	     * Push it on the stack
	     */
	    idl_global->scopes()->push(m);
	  } break;
case 21:
# line 319 "idl.yy"
{
	    idl_global->set_parse_state(IDL_GlobalData::PS_ModuleSqSeen);
	  } break;
case 22:
# line 323 "idl.yy"
{
	    idl_global->set_parse_state(IDL_GlobalData::PS_ModuleBodySeen);
	  } break;
case 23:
# line 327 "idl.yy"
{
	    idl_global->set_parse_state(IDL_GlobalData::PS_ModuleQsSeen);
	    /*
	     * Finished with this module - pop it from the scope stack
	     */
	    idl_global->scopes()->pop();
	  } break;
case 26:
# line 343 "idl.yy"
{
	  UTL_Scope     *s = idl_global->scopes()->top_non_null();
	  AST_Interface *i = NULL;
	  AST_Decl	*v = NULL;
	  UTL_StrList   *p = idl_global->pragmas();
	  AST_Decl	*d = NULL;
	  AST_Interface *fd = NULL;

	  /*
	   * Make a new interface node and add it to its enclosing scope
	   */
	  if (s != NULL && yypvt[-0].ihval != NULL) {
	    i = idl_global->gen()->create_interface(yypvt[-0].ihval->interface_name(),
						    yypvt[-0].ihval->inherits(),
						    yypvt[-0].ihval->n_inherits(),
						    p);
	    if (i != NULL &&
		(d = s->lookup_by_name(i->name(), I_FALSE)) != NULL) {
	      /*
	       * See if we're defining a forward declared interface.
	       */
	      if (d->node_type() == AST_Decl::NT_interface) {
		/*
		 * Narrow to an interface
		 */
		fd = AST_Interface::narrow_from_decl(d);
		/*
		 * Successful?
		 */
		if (fd == NULL) {
		  /*
		   * Should we give an error here?
		   */
		}
		/*
		 * If it is a forward declared interface..
		 */
		else if (!fd->is_defined()) {
		  /*
		   * Check if redefining in same scope
		   */
		  if (fd->defined_in() != s) {
		    idl_global->err()
		       ->error3(UTL_Error::EIDL_SCOPE_CONFLICT,
				i,
				fd,
				ScopeAsDecl(s));
		  }
		  /*
		   * All OK, do the redefinition
		   */
		  else {
		    fd->set_inherits(yypvt[-0].ihval->inherits());
		    fd->set_n_inherits(yypvt[-0].ihval->n_inherits());
		    /*
		     * Update place of definition
		     */
		    fd->set_imported(idl_global->imported());
		    fd->set_in_main_file(idl_global->in_main_file());
		    fd->set_line(idl_global->lineno());
		    fd->set_file_name(idl_global->filename());
		    fd->add_pragmas(p);
		    /*
		     * Use full definition node
		     */
		    delete i;
		    i = fd;
		  }
	        }
	      }
	    }
	    /*
	     * Add the interface to its definition scope
	     */
	    (void) s->fe_add_interface(i);
	  }
	  /*
	   * Push it on the scope stack
	   */
	  idl_global->scopes()->push(i);
        } break;
case 27:
# line 425 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceSqSeen);
	} break;
case 28:
# line 429 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceBodySeen);
	} break;
case 29:
# line 433 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceQsSeen);
	  /*
	   * Done with this interface - pop it off the scopes stack
	   */
	  idl_global->scopes()->pop();
	} break;
case 30:
# line 444 "idl.yy"
{
	   idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceSeen);
	 } break;
case 31:
# line 448 "idl.yy"
{
	   idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceIDSeen);
	   yyval.idval = yypvt[-0].idval;
	 } break;
case 32:
# line 456 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_InheritSpecSeen);
	  /*
	   * Create an AST representation of the information in the header
	   * part of an interface - this representation contains a computed
	   * list of all interfaces which this interface inherits from,
	   * recursively
	   */
	  yyval.ihval = new FE_InterfaceHeader(new UTL_ScopedName(yypvt[-1].idval, NULL), yypvt[-0].nlval);
	} break;
case 33:
# line 470 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_InheritColonSeen);
        } break;
case 34:
# line 474 "idl.yy"
{
	  yyval.nlval = yypvt[-0].nlval;
	} break;
case 35:
# line 478 "idl.yy"
{
	  yyval.nlval = NULL;
	} break;
case 38:
# line 490 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_TypeDeclSeen);
        } break;
case 39:
# line 494 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        } break;
case 40:
# line 498 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_ConstDeclSeen);
        } break;
case 41:
# line 502 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        } break;
case 42:
# line 506 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_ExceptDeclSeen);
        } break;
case 43:
# line 510 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        } break;
case 44:
# line 514 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_AttrDeclSeen);
        } break;
case 45:
# line 518 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        } break;
case 46:
# line 522 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_OpDeclSeen);
        } break;
case 47:
# line 526 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        } break;
case 48:
# line 530 "idl.yy"
{
	  idl_global->err()->syntax_error(idl_global->parse_state());
	} break;
case 49:
# line 534 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
	  yyerrok;
	} break;
case 50:
# line 542 "idl.yy"
{
	  yyval.nlval = new UTL_NameList(yypvt[-1].idlist, yypvt[-0].nlval);
	} break;
case 51:
# line 550 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_SNListCommaSeen);
        } break;
case 52:
# line 554 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_ScopedNameSeen);

	  if (yypvt[-3].nlval == NULL)
	    yyval.nlval = new UTL_NameList(yypvt[-0].idlist, NULL);
	  else {
	    yypvt[-3].nlval->nconc(new UTL_NameList(yypvt[-0].idlist, NULL));
	    yyval.nlval = yypvt[-3].nlval;
	  }
	} break;
case 53:
# line 565 "idl.yy"
{
	  yyval.nlval = NULL;
	} break;
case 54:
# line 572 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_SN_IDSeen);

	  yyval.idlist = new UTL_IdList(yypvt[-0].idval, NULL);
	} break;
case 55:
# line 578 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_ScopeDelimSeen);
        } break;
case 56:
# line 582 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_SN_IDSeen);

	  yyval.idlist = new UTL_IdList(new Identifier(yypvt[-2].strval, 1, 0, I_FALSE),
			      new UTL_IdList(yypvt[-0].idval, NULL));
	} break;
case 57:
# line 590 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_ScopeDelimSeen);
        } break;
case 58:
# line 594 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_SN_IDSeen);

	  yypvt[-3].idlist->nconc(new UTL_IdList(yypvt[-0].idval, NULL));
	  yyval.idlist = yypvt[-3].idlist;
	} break;
case 59:
# line 603 "idl.yy"
{
            yyval.idval = new Identifier(yypvt[-0].strval, 1, 0, I_FALSE);
        } break;
case 60:
# line 610 "idl.yy"
{
	  UTL_Scope		*s = idl_global->scopes()->top_non_null();
	  UTL_ScopedName	*n = new UTL_ScopedName(yypvt[-0].idval, NULL);
	  AST_InterfaceFwd	*f = NULL;
	  UTL_StrList		*p = idl_global->pragmas();

	  idl_global->set_parse_state(IDL_GlobalData::PS_ForwardDeclSeen);
	  /*
	   * Create a node representing a forward declaration of an
	   * interface. Store it in the enclosing scope
	   */
	  if (s != NULL) {
	    f = idl_global->gen()->create_interface_fwd(n, p);
	    (void) s->fe_add_interface_fwd(f);
	  }
	} break;
case 61:
# line 630 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_ConstSeen);
        } break;
case 62:
# line 634 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_ConstTypeSeen);
        } break;
case 63:
# line 638 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_ConstIDSeen);
        } break;
case 64:
# line 642 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_ConstAssignSeen);
        } break;
case 65:
# line 646 "idl.yy"
{
	  UTL_ScopedName	*n = new UTL_ScopedName(yypvt[-4].idval, NULL);
	  UTL_Scope		*s = idl_global->scopes()->top_non_null();
	  AST_Constant		*c = NULL;
	  UTL_StrList		*p = idl_global->pragmas();
	  AST_Decl		*v = NULL;

	  idl_global->set_parse_state(IDL_GlobalData::PS_ConstExprSeen);
	  /*
	   * Create a node representing a constant declaration. Store
	   * it in the enclosing scope
	   */
	  if (yypvt[-0].exval != NULL && s != NULL) {
	    if (yypvt[-0].exval->coerce(yypvt[-6].etval) == NULL)
	      idl_global->err()->coercion_error(yypvt[-0].exval, yypvt[-6].etval);
	    else {
	      c =
		idl_global->gen()->create_constant(yypvt[-6].etval, yypvt[-0].exval, n, p);
	      (void) s->fe_add_constant(c);
	    }
	  }
	} break;
case 71:
# line 677 "idl.yy"
{
	  yyval.etval = AST_Expression::EV_string;
	} break;
case 72:
# line 681 "idl.yy"
{
	  yyval.etval = AST_Expression::EV_wstring;
        } break;
case 73:
# line 685 "idl.yy"
{
	  UTL_Scope		*s = idl_global->scopes()->top_non_null();
	  AST_Decl		*d = NULL;
	  AST_PredefinedType	*c = NULL;
	  AST_Typedef		*t = NULL;

	  /*
	   * If the constant's type is a scoped name, it must resolve
	   * to a scalar constant type
	   */
	  if (s != NULL && (d = s->lookup_by_name(yypvt[-0].idlist, I_TRUE)) != NULL) {
	    /*
	     * Look through typedefs
	     */
	    while (d->node_type() == AST_Decl::NT_typedef) {
	      t = AST_Typedef::narrow_from_decl(d);
	      if (t == NULL)
	        break;
	      d = t->base_type();
	    }
	    if (d == NULL)
	      yyval.etval = AST_Expression::EV_any;
	    else if (d->node_type() == AST_Decl::NT_pre_defined) {
	      c = AST_PredefinedType::narrow_from_decl(d);
	      if (c != NULL) {
	         yyval.etval = idl_global->PredefinedTypeToExprType(c->pt());
	      } else {
	         yyval.etval = AST_Expression::EV_any;
	      }
	    } else if (d->node_type() == AST_Decl::NT_string) {
	      yyval.etval = AST_Expression::EV_string;
	    }
	    else {
              // We should check for wstring here. Not done at the moment
	      // any typedef of wstring used would definitely fail with
	      // a coersion error. Fix this when support for wstring is added.
	      yyval.etval = AST_Expression::EV_any;
	    }
	  } else
	    yyval.etval = AST_Expression::EV_any;
	} break;
case 77:
# line 734 "idl.yy"
{
	  yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_or, yypvt[-2].exval, yypvt[-0].exval);
	} break;
case 79:
# line 742 "idl.yy"
{
	  yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_xor, yypvt[-2].exval, yypvt[-0].exval);
	} break;
case 81:
# line 750 "idl.yy"
{
	  yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_and, yypvt[-2].exval, yypvt[-0].exval);
	} break;
case 83:
# line 758 "idl.yy"
{
	  yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_left,yypvt[-2].exval,yypvt[-0].exval);
	} break;
case 84:
# line 762 "idl.yy"
{
	  yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_right,yypvt[-2].exval,yypvt[-0].exval);
	} break;
case 86:
# line 770 "idl.yy"
{
	  yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_add, yypvt[-2].exval, yypvt[-0].exval);
	} break;
case 87:
# line 774 "idl.yy"
{
	  yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_minus,yypvt[-2].exval,yypvt[-0].exval);
	} break;
case 89:
# line 782 "idl.yy"
{
	  yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_mul, yypvt[-2].exval, yypvt[-0].exval);
	} break;
case 90:
# line 786 "idl.yy"
{
	  yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_div, yypvt[-2].exval, yypvt[-0].exval);
	} break;
case 91:
# line 790 "idl.yy"
{
	  yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_mod, yypvt[-2].exval, yypvt[-0].exval);
	} break;
case 93:
# line 798 "idl.yy"
{
	  yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_u_plus,
					      yypvt[-0].exval,
					      NULL);
	} break;
case 94:
# line 804 "idl.yy"
{
	  yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_u_minus,
					      yypvt[-0].exval,
					      NULL);
	} break;
case 95:
# line 810 "idl.yy"
{
	  yyval.exval = idl_global->gen()->create_expr(AST_Expression::EC_bit_neg,
					      yypvt[-0].exval,
					      NULL);
	} break;
case 96:
# line 819 "idl.yy"
{
	  /*
	   * An expression which is a scoped name is not resolved now,
	   * but only when it is evaluated (such as when it is assigned
	   * as a constant value)
	   */
	  yyval.exval = idl_global->gen()->create_expr(yypvt[-0].idlist);
	} break;
case 98:
# line 829 "idl.yy"
{
	  yyval.exval = yypvt[-1].exval;
	} break;
case 99:
# line 836 "idl.yy"
{
	  yyval.exval = idl_global->gen()->create_expr(yypvt[-0].ival);
	} break;
case 100:
# line 840 "idl.yy"
{
	  yyval.exval = idl_global->gen()->create_expr(yypvt[-0].sval);
	} break;
case 101:
# line 844 "idl.yy"
{
	  yyval.exval = idl_global->gen()->create_expr(yypvt[-0].cval);
	} break;
case 102:
# line 848 "idl.yy"
{
	  yyval.exval = idl_global->gen()->create_expr(yypvt[-0].dval);
	} break;
case 103:
# line 852 "idl.yy"
{
	  yyval.exval = idl_global->gen()->create_expr((idl_bool) I_TRUE,
					    AST_Expression::EV_bool);
	} break;
case 104:
# line 857 "idl.yy"
{
	  yyval.exval = idl_global->gen()->create_expr((idl_bool) I_FALSE,
					    AST_Expression::EV_bool);
	} break;
case 105:
# line 865 "idl.yy"
{
	    yypvt[-0].exval->evaluate(AST_Expression::EK_const);
	    yyval.exval = idl_global->gen()->create_expr(yypvt[-0].exval, AST_Expression::EV_ulong);
	} break;
case 106:
# line 873 "idl.yy"
{
	    idl_global->set_parse_state(IDL_GlobalData::PS_TypedefSeen);
	  } break;
case 111:
# line 884 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_TypeSpecSeen);
        } break;
case 112:
# line 888 "idl.yy"
{
	  UTL_Scope		*s = idl_global->scopes()->top_non_null();
	  UTL_DecllistActiveIterator *l;
	  FE_Declarator		*d = NULL;
	  AST_Typedef		*t = NULL;
	  AST_Decl		*v = NULL;
	  UTL_StrList		*p = idl_global->pragmas();

	  idl_global->set_parse_state(IDL_GlobalData::PS_DeclaratorsSeen);
	  /*
	   * Create a list of type renamings. Add them to the
	   * enclosing scope
	   */
	  if (s != NULL && yypvt[-2].dcval != NULL && yypvt[-0].dlval != NULL) {
	    l = new UTL_DecllistActiveIterator(yypvt[-0].dlval);
	    for (;!(l->is_done()); l->next()) {
	      d = l->item();
	      if (d == NULL) 
		continue;
              AST_Type * tp = d->compose(yypvt[-2].dcval);
              if (tp == NULL) 
		continue;
	      t = idl_global->gen()->create_typedef(tp, d->name(), p);
	      (void) s->fe_add_typedef(t);
	    }
	    delete l;
	  }
	} break;
case 115:
# line 925 "idl.yy"
{
	  yyval.dcval = idl_global->scopes()->bottom()->lookup_primitive_type(yypvt[-0].etval);
	} break;
case 117:
# line 930 "idl.yy"
{
	  UTL_Scope	*s = idl_global->scopes()->top_non_null();
	  AST_Decl	*d = NULL;

	  if (s != NULL)
	    d = s->lookup_by_name(yypvt[-0].idlist, I_TRUE);
	  if (d == NULL)
	    idl_global->err()->lookup_error(yypvt[-0].idlist);
	  yyval.dcval = d;
	} break;
case 130:
# line 965 "idl.yy"
{
	  yyval.dcval = idl_global->scopes()->bottom()->lookup_primitive_type(yypvt[-0].etval);
	} break;
case 132:
# line 970 "idl.yy"
{
	  UTL_Scope	*s = idl_global->scopes()->top_non_null();
	  AST_Decl	*d = NULL;

	  if (s != NULL)
	    d = s->lookup_by_name(yypvt[-0].idlist, I_TRUE);
	  if (d == NULL)
	    idl_global->err()->lookup_error(yypvt[-0].idlist);
	  yyval.dcval = d;
	} break;
case 133:
# line 984 "idl.yy"
{
	  yyval.dlval = new UTL_DeclList(yypvt[-1].deval, yypvt[-0].dlval);
	} break;
case 134:
# line 991 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_DeclsCommaSeen);
        } break;
case 135:
# line 995 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_DeclsDeclSeen);

	  if (yypvt[-3].dlval == NULL)
	    yyval.dlval = new UTL_DeclList(yypvt[-0].deval, NULL);
	  else {
	    yypvt[-3].dlval->nconc(new UTL_DeclList(yypvt[-0].deval, NULL));
	    yyval.dlval = yypvt[-3].dlval;
	  }
	} break;
case 136:
# line 1006 "idl.yy"
{
	  yyval.dlval = NULL;
	} break;
case 139:
# line 1018 "idl.yy"
{
	   yyval.dlval = new UTL_DeclList(yypvt[-1].deval, yypvt[-0].dlval);
	} break;
case 140:
# line 1026 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_DeclsCommaSeen);
	} break;
case 141:
# line 1030 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_DeclsDeclSeen);

	  if (yypvt[-3].dlval == NULL)
	    yyval.dlval = new UTL_DeclList(yypvt[-0].deval, NULL);
	  else {
	    yypvt[-3].dlval->nconc(new UTL_DeclList(yypvt[-0].deval, NULL));
	    yyval.dlval = yypvt[-3].dlval;
	  }
	} break;
case 142:
# line 1041 "idl.yy"
{
	   yyval.dlval = NULL;
	} break;
case 143:
# line 1049 "idl.yy"
{
	  yyval.deval = new FE_Declarator(new UTL_ScopedName(yypvt[-0].idval, NULL),
				 FE_Declarator::FD_simple, NULL);
	} break;
case 144:
# line 1057 "idl.yy"
{
	  yyval.deval = new FE_Declarator(new UTL_ScopedName(yypvt[-0].dcval->local_name(), NULL),
				 FE_Declarator::FD_complex,
				 yypvt[-0].dcval);
	} break;
case 147:
# line 1071 "idl.yy"
{
	  yyval.etval = AST_Expression::EV_long;
	} break;
case 148:
# line 1075 "idl.yy"
{
	  yyval.etval = AST_Expression::EV_longlong;
        } break;
case 149:
# line 1079 "idl.yy"
{
	  yyval.etval = AST_Expression::EV_short;
	} break;
case 150:
# line 1086 "idl.yy"
{
	  yyval.etval = AST_Expression::EV_ulong;
	} break;
case 151:
# line 1090 "idl.yy"
{
	  yyval.etval = AST_Expression::EV_ulonglong;
        } break;
case 152:
# line 1094 "idl.yy"
{
	  yyval.etval = AST_Expression::EV_ushort;
	} break;
case 153:
# line 1101 "idl.yy"
{
	  yyval.etval = AST_Expression::EV_double;
	} break;
case 154:
# line 1105 "idl.yy"
{
	  yyval.etval = AST_Expression::EV_float;
	} break;
case 155:
# line 1109 "idl.yy"
{
	  yyval.etval = AST_Expression::EV_longdouble;
        } break;
case 156:
# line 1116 "idl.yy"
{
	  yyval.etval = AST_Expression::EV_char;
	} break;
case 157:
# line 1120 "idl.yy"
{
	  yyval.etval = AST_Expression::EV_wchar;
        } break;
case 158:
# line 1127 "idl.yy"
{ 
          yyval.etval = AST_Expression::EV_octet;
	} break;
case 159:
# line 1134 "idl.yy"
{ 
	  yyval.etval = AST_Expression::EV_bool;
        } break;
case 160:
# line 1141 "idl.yy"
{
	  yyval.etval = AST_Expression::EV_any;
	} break;
case 161:
# line 1148 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_StructSeen);
        } break;
case 162:
# line 1152 "idl.yy"
{
	  UTL_Scope		*s = idl_global->scopes()->top_non_null();
	  UTL_ScopedName	*n = new UTL_ScopedName(yypvt[-0].idval, NULL);
	  AST_Structure		*d = NULL;
	  UTL_StrList		*p = idl_global->pragmas();
	  AST_Decl		*v = NULL;

	  idl_global->set_parse_state(IDL_GlobalData::PS_StructIDSeen);
	  /*
	   * Create a node representing a struct declaration. Add it
	   * to the enclosing scope
	   */
	  if (s != NULL) {
	    d = idl_global->gen()->create_structure(n, p);
	    (void) s->fe_add_structure(d);
	  }
	  /*
	   * Push the scope of the struct on the scopes stack
	   */
	  idl_global->scopes()->push(d);
	} break;
case 163:
# line 1174 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_StructSqSeen);
        } break;
case 164:
# line 1178 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_StructBodySeen);
        } break;
case 165:
# line 1182 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_StructQsSeen);
	  /*
	   * Done with this struct. Pop its scope off the scopes stack
	   */
	  if (idl_global->scopes()->top() == NULL)
	    yyval.dcval = NULL;
	  else {
	    yyval.dcval =
	      AST_Structure::narrow_from_scope(
				   idl_global->scopes()->top_non_null());
	    idl_global->scopes()->pop();
	  }
	} break;
case 169:
# line 1207 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_MemberTypeSeen);
        } break;
case 170:
# line 1211 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_MemberDeclsSeen);
        } break;
case 171:
# line 1215 "idl.yy"
{
	  UTL_Scope		*s = idl_global->scopes()->top_non_null();
	  UTL_DecllistActiveIterator *l = NULL;
	  FE_Declarator		*d = NULL;
	  AST_Field		*f = NULL;
	  UTL_StrList		*p = idl_global->pragmas();

	  idl_global->set_parse_state(IDL_GlobalData::PS_MemberDeclsCompleted);
	  /*
	   * Check for illegal recursive use of type
	   */
	  if (yypvt[-4].dcval != NULL && AST_illegal_recursive_type(yypvt[-4].dcval))
	    idl_global->err()->error1(UTL_Error::EIDL_RECURSIVE_TYPE, yypvt[-4].dcval);
	  /*
	   * Create a node representing a struct or exception member
	   * Add it to the enclosing scope
	   */
	  else if (s != NULL && yypvt[-4].dcval != NULL && yypvt[-2].dlval != NULL) {
	    l = new UTL_DecllistActiveIterator(yypvt[-2].dlval);
	    for (;!(l->is_done()); l->next()) {
	      d = l->item();
	      if (d == NULL) 
		continue;
 	      AST_Type *tp = d->compose(yypvt[-4].dcval);
	      if (tp == NULL) 
		continue;
	      f = idl_global->gen()->create_field(tp, d->name(), p);
	      (void) s->fe_add_field(f);
	    }
	    delete l;
	  }
	} break;
case 172:
# line 1248 "idl.yy"
{
	  idl_global->err()->syntax_error(idl_global->parse_state());
	} break;
case 173:
# line 1252 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
	  yyerrok;
	} break;
case 174:
# line 1260 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_UnionSeen);
        } break;
case 175:
# line 1264 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_UnionIDSeen);
        } break;
case 176:
# line 1268 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_SwitchSeen);
        } break;
case 177:
# line 1272 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_SwitchOpenParSeen);
        } break;
case 178:
# line 1276 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_SwitchTypeSeen);
        } break;
case 179:
# line 1280 "idl.yy"
{
	  UTL_Scope		*s = idl_global->scopes()->top_non_null();
	  UTL_ScopedName	*n = new UTL_ScopedName(yypvt[-8].idval, NULL);
	  AST_Union		*u = NULL;
	  AST_Decl		*v = NULL;
	  UTL_StrList		*p = idl_global->pragmas();

	  idl_global->set_parse_state(IDL_GlobalData::PS_SwitchCloseParSeen);
	  /*
	   * Create a node representing a union. Add it to its enclosing
	   * scope
	   */
	  if (yypvt[-2].dcval != NULL && s != NULL) {
 	    AST_ConcreteType    *tp = AST_ConcreteType::narrow_from_decl(yypvt[-2].dcval);
            if (tp == NULL) {
              idl_global->err()->not_a_type(yypvt[-2].dcval);
            } else {
	      u = idl_global->gen()->create_union(tp, n, p);
	      (void) s->fe_add_union(u);
 	    }
	  }
	  /*
	   * Push the scope of the union on the scopes stack
	   */
	  idl_global->scopes()->push(u);
	} break;
case 180:
# line 1307 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_UnionSqSeen);
        } break;
case 181:
# line 1311 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_UnionBodySeen);
        } break;
case 182:
# line 1315 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_UnionQsSeen);
	  /*
	   * Done with this union. Pop its scope from the scopes stack
	   */
	  if (idl_global->scopes()->top() == NULL)
	    yyval.dcval = NULL;
	  else {
	    yyval.dcval =
	      AST_Union::narrow_from_scope(
				idl_global->scopes()->top_non_null());
	    idl_global->scopes()->pop();
	  }
	} break;
case 183:
# line 1333 "idl.yy"
{
	  yyval.dcval = idl_global->scopes()->bottom()->lookup_primitive_type(yypvt[-0].etval);
	} break;
case 184:
# line 1337 "idl.yy"
{
	  yyval.dcval = idl_global->scopes()->bottom()->lookup_primitive_type(yypvt[-0].etval);
	} break;
case 185:
# line 1341 "idl.yy"
{
	  yyval.dcval = idl_global->scopes()->bottom()->lookup_primitive_type(yypvt[-0].etval);
	} break;
case 186:
# line 1345 "idl.yy"
{
	  yyval.dcval = idl_global->scopes()->bottom()->lookup_primitive_type(yypvt[-0].etval);
	} break;
case 188:
# line 1350 "idl.yy"
{
	  UTL_Scope		*s = idl_global->scopes()->top_non_null();
	  AST_Decl		*d = NULL;
	  AST_PredefinedType	*p = NULL;
	  AST_Typedef		*t = NULL;
	  long			found = I_FALSE;

	  /*
	   * The discriminator is a scoped name. Try to resolve to
	   * one of the scalar types or to an enum. Thread through
	   * typedef's to arrive at the base type at the end of the
	   * chain
	   */
	  if (s != NULL && (d = s->lookup_by_name(yypvt[-0].idlist, I_TRUE)) != NULL) {
	    while (!found) {
	      switch (d->node_type()) {
	      case AST_Decl::NT_enum:
		yyval.dcval = d;
		found = I_TRUE;
		break;
	      case AST_Decl::NT_pre_defined:
		p = AST_PredefinedType::narrow_from_decl(d);
		if (p != NULL) {
		  switch (p->pt()) {
		  case AST_PredefinedType::PT_long:
		  case AST_PredefinedType::PT_ulong:
		  case AST_PredefinedType::PT_longlong:
		  case AST_PredefinedType::PT_ulonglong:
		  case AST_PredefinedType::PT_short:
		  case AST_PredefinedType::PT_char:
		  case AST_PredefinedType::PT_wchar:
		  case AST_PredefinedType::PT_octet:
		  case AST_PredefinedType::PT_boolean:
		    yyval.dcval = p;
		    found = I_TRUE;
		    break;
		  default:
		    yyval.dcval = NULL;
		    found = I_TRUE;
		    break;
		  }
		}
		break;
	      case AST_Decl::NT_typedef:
		t = AST_Typedef::narrow_from_decl(d);
		if (t != NULL) d = t->base_type();
		break;
	      default:
		yyval.dcval = NULL;
		found = I_TRUE;
		break;
	      }
	    }
	  } else
	    yyval.dcval = NULL;

	  if (yyval.dcval == NULL)
	    idl_global->err()->lookup_error(yypvt[-0].idlist);
	} break;
case 192:
# line 1420 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_UnionLabelSeen);
        } break;
case 193:
# line 1424 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_UnionElemSeen);
        } break;
case 194:
# line 1428 "idl.yy"
{
	  UTL_Scope		*s = idl_global->scopes()->top_non_null();
	  UTL_LabellistActiveIterator *l = NULL;
	  AST_UnionLabel	*d = NULL;
	  AST_UnionBranch	*b = NULL;
	  AST_Field		*f = yypvt[-2].ffval;

	  idl_global->set_parse_state(IDL_GlobalData::PS_UnionElemCompleted);
	  /*
	   * Create several nodes representing branches of a union.
	   * Add them to the enclosing scope (the union scope)
	   */
	  if (s != NULL && yypvt[-4].llval != NULL && yypvt[-2].ffval != NULL) {
	    l = new UTL_LabellistActiveIterator(yypvt[-4].llval);
	    for (;!(l->is_done()); l->next()) {
	      d = l->item();
	      if (d == NULL)
		continue;
	      b = idl_global->gen()->create_union_branch(d,
						      f->field_type(),
						      f->name(),
						      f->pragmas());
	      (void) s->fe_add_union_branch(b);
	    }
	    delete l;
	  }
	} break;
case 195:
# line 1456 "idl.yy"
{
	  idl_global->err()->syntax_error(idl_global->parse_state());
	} break;
case 196:
# line 1460 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
	  yyerrok;
	} break;
case 197:
# line 1468 "idl.yy"
{
	  yyval.llval = new UTL_LabelList(yypvt[-1].ulval, yypvt[-0].llval);
	} break;
case 198:
# line 1475 "idl.yy"
{
	  if (yypvt[-1].llval == NULL)
	    yyval.llval = new UTL_LabelList(yypvt[-0].ulval, NULL);
	  else {
	    yypvt[-1].llval->nconc(new UTL_LabelList(yypvt[-0].ulval, NULL));
	    yyval.llval = yypvt[-1].llval;
	  }
	} break;
case 199:
# line 1484 "idl.yy"
{
	  yyval.llval = NULL;
	} break;
case 200:
# line 1491 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_DefaultSeen);
        } break;
case 201:
# line 1495 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_LabelColonSeen);

	  yyval.ulval = idl_global->gen()->
	            create_union_label(AST_UnionLabel::UL_default,
				       NULL);
	} break;
case 202:
# line 1503 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_CaseSeen);
        } break;
case 203:
# line 1507 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_LabelExprSeen);
        } break;
case 204:
# line 1511 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_LabelColonSeen);

	  yyval.ulval = idl_global->gen()->create_union_label(AST_UnionLabel::UL_label,
						     yypvt[-2].exval);
	} break;
case 205:
# line 1521 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_UnionElemTypeSeen);
        } break;
case 206:
# line 1525 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_UnionElemDeclSeen);
	  /*
	   * Check for illegal recursive use of type
	   */
	  if (yypvt[-2].dcval != NULL && AST_illegal_recursive_type(yypvt[-2].dcval))
	    idl_global->err()->error1(UTL_Error::EIDL_RECURSIVE_TYPE, yypvt[-2].dcval);
	  /*
	   * Create a field in a union branch
	   */
	  else if (yypvt[-2].dcval == NULL || yypvt[-0].deval == NULL)
	    yyval.ffval = NULL;
	  else {
	    AST_Type *tp = yypvt[-0].deval->compose(yypvt[-2].dcval);
	    if (tp == NULL)
	      yyval.ffval = NULL;
 	    else
	      yyval.ffval = idl_global->gen()->create_field(tp,
			        		   yypvt[-0].deval->name(),
			        		   idl_global->pragmas());
	  }
	} break;
case 207:
# line 1551 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_EnumSeen);
        } break;
case 208:
# line 1555 "idl.yy"
{
	  UTL_Scope		*s = idl_global->scopes()->top_non_null();
	  UTL_ScopedName	*n = new UTL_ScopedName(yypvt[-0].idval, NULL);
	  AST_Enum		*e = NULL;
	  AST_Decl		*v = NULL;
	  UTL_StrList		*p = idl_global->pragmas();

	  idl_global->set_parse_state(IDL_GlobalData::PS_EnumIDSeen);
	  /*
	   * Create a node representing an enum and add it to its
	   * enclosing scope
	   */
	  if (s != NULL) {
	    e = idl_global->gen()->create_enum(n, p);
	    /*
	     * Add it to its defining scope
	     */
	    (void) s->fe_add_enum(e);
	  }
	  /*
	   * Push the enum scope on the scopes stack
	   */
	  idl_global->scopes()->push(e);
	} break;
case 209:
# line 1580 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_EnumSqSeen);
        } break;
case 210:
# line 1584 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_EnumBodySeen);
        } break;
case 211:
# line 1588 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_EnumQsSeen);
	  /*
	   * Done with this enum. Pop its scope from the scopes stack
	   */
	  if (idl_global->scopes()->top() == NULL)
	    yyval.dcval = NULL;
	  else {
	    yyval.dcval = AST_Enum::narrow_from_scope(idl_global->scopes()->top_non_null());
	    idl_global->scopes()->pop();
	  }
	} break;
case 213:
# line 1607 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_EnumCommaSeen);
        } break;
case 216:
# line 1616 "idl.yy"
{
	  UTL_Scope		*s = idl_global->scopes()->top_non_null();
	  UTL_ScopedName	*n =
		new UTL_ScopedName(new Identifier(yypvt[-0].strval, 1, 0, I_FALSE), NULL);
	  AST_EnumVal		*e = NULL;
	  AST_Enum		*c = NULL;
	  UTL_StrList		*p = idl_global->pragmas();

	  /*
	   * Create a node representing one enumerator in an enum
	   * Add it to the enclosing scope (the enum scope)
	   */
	  if (s != NULL && s->scope_node_type() == AST_Decl::NT_enum) {
	    c = AST_Enum::narrow_from_scope(s);
	    if (c != NULL) 
	      e = idl_global->gen()->create_enum_val(c->next_enum_val(), n, p);
	    (void) s->fe_add_enum_val(e);
	  }
	} break;
case 217:
# line 1640 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_SequenceCommaSeen);
        } break;
case 218:
# line 1644 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_SequenceExprSeen);
        } break;
case 219:
# line 1648 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_SequenceQsSeen);
	  /*
	   * Remove sequence marker from scopes stack
	   */
	  if (idl_global->scopes()->top() == NULL)
	    idl_global->scopes()->pop();
	  /*
	   * Create a node representing a sequence
	   */
	  if (yypvt[-2].exval == NULL || yypvt[-2].exval->coerce(AST_Expression::EV_ulong) == NULL) {
	    idl_global->err()->coercion_error(yypvt[-2].exval, AST_Expression::EV_ulong);
	    yyval.dcval = NULL;
	  } else if (yypvt[-5].dcval == NULL) {
	    yyval.dcval = NULL;
	  } else {
 	    AST_Type *tp = AST_Type::narrow_from_decl(yypvt[-5].dcval); 
	    if (tp == NULL) 
	      yyval.dcval = NULL;
	    else {
	      yyval.dcval = idl_global->gen()->create_sequence(yypvt[-2].exval, tp);
	      /*
	       * Add this AST_Sequence to the types defined in the global scope
	       */
	      (void) idl_global->root()
		        ->fe_add_sequence(AST_Sequence::narrow_from_decl(yyval.dcval));
 	    }
	  }
	} break;
case 220:
# line 1679 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_SequenceQsSeen);
	  /*
	   * Remove sequence marker from scopes stack
	   */
	  if (idl_global->scopes()->top() == NULL)
	    idl_global->scopes()->pop();
	  /*
	   * Create a node representing a sequence
	   */
	  if (yypvt[-1].dcval == NULL)
	    yyval.dcval = NULL;
	  else {
	    AST_Type *tp = AST_Type::narrow_from_decl(yypvt[-1].dcval);
	    if (tp == NULL)
	      yyval.dcval = NULL;
            else {
	      yyval.dcval =
	        idl_global->gen()->create_sequence(
		  	     idl_global->gen()->create_expr((unsigned long) 0),
			     tp);
	      /*
	       * Add this AST_Sequence to the types defined in the global scope
	       */
	      (void) idl_global->root()
		        ->fe_add_sequence(AST_Sequence::narrow_from_decl(yyval.dcval));
	    }
	  }
	} break;
case 221:
# line 1712 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_SequenceSeen);
	  /*
	   * Push a sequence marker on scopes stack
	   */
	  idl_global->scopes()->push(NULL);
	} break;
case 222:
# line 1720 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_SequenceSqSeen);
        } break;
case 223:
# line 1724 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_SequenceTypeSeen);
	  yyval.dcval = yypvt[-0].dcval;
        } break;
case 224:
# line 1733 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_StringSqSeen);
        } break;
case 225:
# line 1737 "idl.yy"
{
	   idl_global->set_parse_state(IDL_GlobalData::PS_StringExprSeen);
        } break;
case 226:
# line 1741 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_StringQsSeen);
	  /*
	   * Create a node representing a string
	   */
	  if (yypvt[-2].exval == NULL || yypvt[-2].exval->coerce(AST_Expression::EV_ulong) == NULL) {
	    idl_global->err()->coercion_error(yypvt[-2].exval, AST_Expression::EV_ulong);
	    yyval.dcval = NULL;
	  } else {
	    yyval.dcval = idl_global->gen()->create_string(yypvt[-2].exval);
	    /*
	     * Add this AST_String to the types defined in the global scope
	     */
	    (void) idl_global->root()
		      ->fe_add_string(AST_String::narrow_from_decl(yyval.dcval));
	  }
	} break;
case 227:
# line 1759 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_StringCompleted);
	  /*
	   * Create a node representing a string
	   */
	  yyval.dcval =
	    idl_global->gen()->create_string(
			 idl_global->gen()->create_expr((unsigned long) 0));
	  /*
	   * Add this AST_String to the types defined in the global scope
	   */
	  (void) idl_global->root()
                    ->fe_add_string(AST_String::narrow_from_decl(yyval.dcval));
	} break;
case 228:
# line 1777 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_StringSeen);
        } break;
case 229:
# line 1785 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_StringSqSeen);
        } break;
case 230:
# line 1789 "idl.yy"
{
	   idl_global->set_parse_state(IDL_GlobalData::PS_StringExprSeen);
        } break;
case 231:
# line 1793 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_StringQsSeen);
	  /*
	   * Create a node representing a string
	   */
	  if (yypvt[-2].exval == NULL || yypvt[-2].exval->coerce(AST_Expression::EV_ulong) == NULL) {
	    idl_global->err()->coercion_error(yypvt[-2].exval, AST_Expression::EV_ulong);
	    yyval.dcval = NULL;
	  } else {
	    yyval.dcval = idl_global->gen()->create_wstring(yypvt[-2].exval);
	    /*
	     * Add this AST_String to the types defined in the global scope
	     */
	    (void) idl_global->root()
		      ->fe_add_string(AST_String::narrow_from_decl(yyval.dcval));
	  }
	} break;
case 232:
# line 1811 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_StringCompleted);
	  /*
	   * Create a node representing a string
	   */
	  yyval.dcval =
	    idl_global->gen()->create_wstring(
			 idl_global->gen()->create_expr((unsigned long) 0));
	  /*
	   * Add this AST_String to the types defined in the global scope
	   */
	  (void) idl_global->root()
                    ->fe_add_string(AST_String::narrow_from_decl(yyval.dcval));
	} break;
case 233:
# line 1829 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_StringSeen);
        } break;
case 234:
# line 1836 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_ArrayIDSeen);
        } break;
case 235:
# line 1840 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_ArrayCompleted);
	  /*
	   * Create a node representing an array
	   */
	  if (yypvt[-0].elval != NULL) {
	     yyval.dcval = idl_global->gen()->create_array(new UTL_ScopedName(yypvt[-2].idval, NULL),
						  yypvt[-0].elval->length(), yypvt[-0].elval);
	  }
	} break;
case 236:
# line 1854 "idl.yy"
{
	  yyval.elval = new UTL_ExprList(yypvt[-1].exval, yypvt[-0].elval);
	} break;
case 237:
# line 1861 "idl.yy"
{
	  if (yypvt[-1].elval == NULL)
	    yyval.elval = new UTL_ExprList(yypvt[-0].exval, NULL);
	  else {
	    yypvt[-1].elval->nconc(new UTL_ExprList(yypvt[-0].exval, NULL));
	    yyval.elval = yypvt[-1].elval;
	  }
	} break;
case 238:
# line 1870 "idl.yy"
{
	  yyval.elval = NULL;
	} break;
case 239:
# line 1877 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_DimSqSeen);
        } break;
case 240:
# line 1881 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_DimExprSeen);
        } break;
case 241:
# line 1885 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_DimQsSeen);
	  /*
	   * Array dimensions are expressions which must be coerced to
	   * positive integers
	   */
	  if (yypvt[-2].exval == NULL || yypvt[-2].exval->coerce(AST_Expression::EV_ulong) == NULL) {
	    idl_global->err()->coercion_error(yypvt[-2].exval, AST_Expression::EV_ulong);
	    yyval.exval = NULL;
	  } else
	    yyval.exval = yypvt[-2].exval;
	} break;
case 242:
# line 1902 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_AttrSeen);
        } break;
case 243:
# line 1906 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_AttrTypeSeen);
        } break;
case 244:
# line 1910 "idl.yy"
{
	  UTL_Scope		*s = idl_global->scopes()->top_non_null();
	  UTL_DecllistActiveIterator *l = NULL;
	  AST_Attribute		*a = NULL;
	  FE_Declarator		*d = NULL;
	  UTL_StrList		*p = idl_global->pragmas();

	  idl_global->set_parse_state(IDL_GlobalData::PS_AttrCompleted);
	  /*
	   * Create nodes representing attributes and add them to the
	   * enclosing scope
	   */
	  if (s != NULL && yypvt[-2].dcval != NULL && yypvt[-0].dlval != NULL) {
	    l = new UTL_DecllistActiveIterator(yypvt[-0].dlval);
	    for (;!(l->is_done()); l->next()) {
	      d = l->item();
	      if (d == NULL)
		continue;
	      AST_Type *tp = d->compose(yypvt[-2].dcval);
 	      if (tp == NULL)
	 	continue;
	      a = idl_global->gen()->create_attribute(yypvt[-5].bval, tp, d->name(), p);
	      /*
	       * Add one attribute to the enclosing scope
	       */
	      (void) s->fe_add_attribute(a);
	    }
	    delete l;
	  }
	} break;
case 245:
# line 1944 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_AttrROSeen);
	  yyval.bval = I_TRUE;
	} break;
case 246:
# line 1949 "idl.yy"
{
	  yyval.bval = I_FALSE;
	} break;
case 247:
# line 1956 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_ExceptSeen);
	} break;
case 248:
# line 1960 "idl.yy"
{
	  UTL_Scope		*s = idl_global->scopes()->top_non_null();
	  UTL_ScopedName	*n = new UTL_ScopedName(yypvt[-0].idval, NULL);
	  AST_Exception		*e = NULL;
	  UTL_StrList		*p = idl_global->pragmas();
	  AST_Decl		*v = NULL;

	  idl_global->set_parse_state(IDL_GlobalData::PS_ExceptIDSeen);
	  /*
	   * Create a node representing an exception and add it to
	   * the enclosing scope
	   */
	  if (s != NULL) {
	    e = idl_global->gen()->create_exception(n, p);
	    (void) s->fe_add_exception(e);
	  }
	  /*
	   * Push the exception scope on the scope stack
	   */
	  idl_global->scopes()->push(e);
	} break;
case 249:
# line 1982 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_ExceptSqSeen);
        } break;
case 250:
# line 1986 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_ExceptBodySeen);
        } break;
case 251:
# line 1990 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_ExceptQsSeen);
	  /*
	   * Done with this exception. Pop its scope from the scope stack
	   */
	  idl_global->scopes()->pop();
	} break;
case 252:
# line 2002 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_OpTypeSeen);
        } break;
case 253:
# line 2006 "idl.yy"
{
	  UTL_Scope		*s = idl_global->scopes()->top_non_null();
	  UTL_ScopedName	*n =
		new UTL_ScopedName(new Identifier(yypvt[-0].strval, 1, 0, I_FALSE), NULL);
	  AST_Operation		*o = NULL;
	  UTL_StrList		*p = idl_global->pragmas();

	  idl_global->set_parse_state(IDL_GlobalData::PS_OpIDSeen);
	  /*
	   * Create a node representing an operation on an interface
	   * and add it to its enclosing scope
	   */
	  if (s != NULL && yypvt[-2].dcval != NULL) {
	    AST_Type *tp = AST_Type::narrow_from_decl(yypvt[-2].dcval);
            if (tp == NULL) {
              idl_global->err()->not_a_type(yypvt[-2].dcval);
            } else if (tp->node_type() == AST_Decl::NT_except) {
              idl_global->err()->not_a_type(yypvt[-2].dcval);
            } else {
	      o = idl_global->gen()->create_operation(tp, yypvt[-3].ofval, n, p);
	      (void) s->fe_add_operation(o);
	    }
	  }
	  /*
	   * Push the operation scope onto the scopes stack
	   */
	  idl_global->scopes()->push(o);
	} break;
case 254:
# line 2035 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_OpParsCompleted);
        } break;
case 255:
# line 2039 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_OpRaiseCompleted);
        } break;
case 256:
# line 2043 "idl.yy"
{
	  UTL_Scope		*s = idl_global->scopes()->top_non_null();
	  AST_Operation		*o = NULL;

	  idl_global->set_parse_state(IDL_GlobalData::PS_OpCompleted);
	  /*
	   * Add exceptions and context to the operation
	   */
	  if (s != NULL && s->scope_node_type() == AST_Decl::NT_op) {
	    o = AST_Operation::narrow_from_scope(s);

	    if (yypvt[-2].nlval != NULL && o != NULL)
	      (void) o->fe_add_exceptions(yypvt[-2].nlval);
	    if (yypvt[-0].slval != NULL)
	      (void) o->fe_add_context(yypvt[-0].slval);
	  }
	  /*
	   * Done with this operation. Pop its scope from the scopes stack
	   */
	  idl_global->scopes()->pop();
	} break;
case 257:
# line 2068 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_OpAttrSeen);
	  yyval.ofval = AST_Operation::OP_oneway;
	} break;
case 258:
# line 2073 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_OpAttrSeen);
	  yyval.ofval = AST_Operation::OP_idempotent;
	} break;
case 259:
# line 2078 "idl.yy"
{
	  yyval.ofval = AST_Operation::OP_noflags;
	} break;
case 261:
# line 2086 "idl.yy"
{
	  yyval.dcval =
	    idl_global->scopes()->bottom()
	       ->lookup_primitive_type(AST_Expression::EV_void);
	} break;
case 262:
# line 2095 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_OpSqSeen);
        } break;
case 263:
# line 2099 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_OpQsSeen);
        } break;
case 264:
# line 2103 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_OpSqSeen);
        } break;
case 265:
# line 2108 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_OpQsSeen);
        } break;
case 266:
# line 2112 "idl.yy"
{
           idl_global->err()->syntax_error(idl_global->parse_state());
        } break;
case 268:
# line 2122 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_OpParCommaSeen);
        } break;
case 271:
# line 2131 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_OpParDirSeen);
        } break;
case 272:
# line 2135 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_OpParTypeSeen);
        } break;
case 273:
# line 2139 "idl.yy"
{
	  UTL_Scope		*s = idl_global->scopes()->top_non_null();
	  AST_Argument		*a = NULL;
	  UTL_StrList		*p = idl_global->pragmas();

	  idl_global->set_parse_state(IDL_GlobalData::PS_OpParDeclSeen);
	  /*
	   * Create a node representing an argument to an operation
	   * Add it to the enclosing scope (the operation scope)
	   */
	  if (yypvt[-2].dcval != NULL && yypvt[-0].deval != NULL && s != NULL) {
	    AST_Type *tp = yypvt[-0].deval->compose(yypvt[-2].dcval);
	    if (tp != NULL) {
	      a = idl_global->gen()->create_argument(yypvt[-4].dival, tp, yypvt[-0].deval->name(), p);
	      (void) s->fe_add_argument(a);
	    }
	  }
	} break;
case 274:
# line 2161 "idl.yy"
{
	  yyval.dival = AST_Argument::dir_IN;
	} break;
case 275:
# line 2165 "idl.yy"
{
	  yyval.dival = AST_Argument::dir_OUT;
	} break;
case 276:
# line 2169 "idl.yy"
{
	  yyval.dival = AST_Argument::dir_INOUT;
	} break;
case 277:
# line 2176 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_OpRaiseSeen);
        } break;
case 278:
# line 2180 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_OpRaiseSqSeen);
        } break;
case 279:
# line 2185 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_OpRaiseQsSeen);
	  yyval.nlval = yypvt[-1].nlval;
	} break;
case 280:
# line 2190 "idl.yy"
{
	  yyval.nlval = NULL;
	} break;
case 281:
# line 2197 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_OpContextSeen);
        } break;
case 282:
# line 2201 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_OpContextSqSeen);
        } break;
case 283:
# line 2206 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_OpContextQsSeen);
	  yyval.slval = yypvt[-1].slval;
	} break;
case 284:
# line 2211 "idl.yy"
{
	  yyval.slval = NULL;
	} break;
case 285:
# line 2218 "idl.yy"
{
	  yyval.slval = new UTL_StrList(yypvt[-1].sval, yypvt[-0].slval);
	} break;
case 286:
# line 2226 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_OpContextCommaSeen);
        } break;
case 287:
# line 2230 "idl.yy"
{
	  if (yypvt[-3].slval == NULL)
	    yyval.slval = new UTL_StrList(yypvt[-0].sval, NULL);
	  else {
	    yypvt[-3].slval->nconc(new UTL_StrList(yypvt[-0].sval, NULL));
	    yyval.slval = yypvt[-3].slval;
	  }
	} break;
case 288:
# line 2239 "idl.yy"
{
	  yyval.slval = NULL;
	} break;
case 289:
# line 2245 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_PragmaVersionSeen);
	} break;
case 290:
# line 2249 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_ScopedNameSeen);
        } break;
case 291:
# line 2253 "idl.yy"
{
          UTL_Scope	*s = idl_global->scopes()->top_non_null();
	  AST_Decl	*d = NULL;

	  if (s != NULL)
	    d = s->lookup_by_name(yypvt[-2].idlist, I_TRUE);
	  if (d == NULL) {
	    idl_global->err()->lookup_error(yypvt[-2].idlist);
	  }
	  else {
	    d->add_pragmas(new UTL_StrList(new String("version"),
					   new UTL_StrList(new String(yytext),
							   NULL
							   )
					   )
			   );
	  }
	} break;
case 292:
# line 2274 "idl.yy"
{	
	  idl_global->set_parse_state(IDL_GlobalData::PS_PragmaIDSeen);
        } break;
case 293:
# line 2278 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_ScopedNameSeen);
        } break;
case 294:
# line 2282 "idl.yy"
{
          UTL_Scope	*s = idl_global->scopes()->top_non_null();
	  AST_Decl	*d = NULL;

	  if (s != NULL)
	    d = s->lookup_by_name(yypvt[-2].idlist, I_TRUE);
	  if (d == NULL) {
	    idl_global->err()->lookup_error(yypvt[-2].idlist);
	  }
	  else {
	    d->add_pragmas(new UTL_StrList(new String("ID"),
					   new UTL_StrList(new String(yypvt[-0].sval),
							   NULL
							   )
					   )
			   );
	  }
	} break;
case 295:
# line 2303 "idl.yy"
{
	  idl_global->set_parse_state(IDL_GlobalData::PS_PragmaPrefixSeen);
        } break;
case 296:
# line 2307 "idl.yy"
{
	  UTL_Scope *s = idl_global->scopes()->top_non_null();
	  AST_Decl  *d = ScopeAsDecl(s);
	  d->add_pragmas(new UTL_StrList(new String("prefix"),
					 new UTL_StrList(new String(yypvt[-0].sval),
							 NULL
							 )
					 )
			 );
	} break;
# line	532 "/usr/ccs/bin/yaccpar"
	}
	goto yystack;		/* reset registers in driver code */
}

#ifdef __VMS
# line 3364 "y_tab.cc"
#endif

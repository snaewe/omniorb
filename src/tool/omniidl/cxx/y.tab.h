typedef union {
  char*                    id_val;
  int                      int_val;
  _CORBA_ULong             ulong_val;
  IdlIntLiteral            int_literal_val;
#ifndef __VMS
  IdlFloatLiteral          float_literal_val;
#else
  double                   float_literal_val;
#endif
  char                     char_val;
  char*                    string_val;
  _CORBA_WChar             wchar_val;
  _CORBA_WChar*            wstring_val;
  _CORBA_Boolean           boolean_val;
  int                      fixed_val; // ***
  IdlType*                 type_val;
  TypeSpec*                type_spec_val;
  IdlExpr*                 expr_val;
  ScopedName*              scopedname_val;
  Decl*                    decl_val;
  Module*                  module_val;
  Interface*               interface_val;
  InheritSpec*             inheritspec_val;
  Forward*                 forward_val;
  Const*                   const_val;
  Typedef*                 typedef_val;
  Struct*                  struct_val;
  Exception*               exception_val;
  Member*                  member_val;
  Declarator*              declarator_val;
  Union*                   union_val;
  UnionCase*               union_case_val;
  CaseLabel*               case_label_val;
  ValueBase*               value_base_val;
  Value*                   value_val;
  ValueForward*            value_forward_val;
  ValueBox*                value_box_val;
  ValueAbs*                value_abs_val;
  ValueInheritSpec*        valueinheritspec_val;
  ValueInheritSupportSpec* valueinheritsupportspec_val;
  StateMember*             statemember_val;
  Factory*                 factory_val;
  Enumerator*              enumerator_val;
  Enum*                    enum_val;
  ArraySize*               array_size_val;
  Attribute*               attribute_val;
  Operation*               operation_val;
  Parameter*               parameter_val;
  RaisesSpec*              raisesspec_val;
  ContextSpec*             contextspec_val;
} YYSTYPE;
#define	IDENTIFIER	257
#define	ABSTRACT	258
#define	ANY	259
#define	ATTRIBUTE	260
#define	BOOLEAN	261
#define	CASE	262
#define	CHAR	263
#define	CONST	264
#define	CONTEXT	265
#define	CUSTOM	266
#define	DEFAULT	267
#define	DOUBLE	268
#define	ENUM	269
#define	EXCEPTION	270
#define	FACTORY	271
#define	FALSE_	272
#define	FIXED	273
#define	FLOAT	274
#define	IN	275
#define	INOUT	276
#define	INTERFACE	277
#define	LONG	278
#define	MODULE	279
#define	NATIVE	280
#define	OBJECT	281
#define	OCTET	282
#define	ONEWAY	283
#define	OUT	284
#define	PRIVATE	285
#define	PUBLIC	286
#define	RAISES	287
#define	READONLY	288
#define	SEQUENCE	289
#define	SHORT	290
#define	STRING	291
#define	STRUCT	292
#define	SUPPORTS	293
#define	SWITCH	294
#define	TRUE_	295
#define	TRUNCATABLE	296
#define	TYPEDEF	297
#define	UNION	298
#define	UNSIGNED	299
#define	VALUEBASE	300
#define	VALUETYPE	301
#define	VOID	302
#define	WCHAR	303
#define	WSTRING	304
#define	PRAGMA	305
#define	PRAGMA_PREFIX	306
#define	PRAGMA_ID	307
#define	PRAGMA_VERSION	308
#define	OMNI_PRAGMA	309
#define	END_PRAGMA	310
#define	UNKNOWN_PRAGMA_BODY	311
#define	INTEGER_LITERAL	312
#define	CHARACTER_LITERAL	313
#define	WIDE_CHARACTER_LITERAL	314
#define	FLOATING_PT_LITERAL	315
#define	STRING_LITERAL	316
#define	WIDE_STRING_LITERAL	317
#define	FIXED_PT_LITERAL	318
#define	SCOPE_DELIM	319
#define	LEFT_SHIFT	320
#define	RIGHT_SHIFT	321


extern YYSTYPE yylval;

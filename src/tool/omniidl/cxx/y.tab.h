typedef union {
  char*                    id_val;
  int                      int_val;
  _CORBA_ULong             ulong_val;
  IdlIntLiteral            int_literal_val;
  IdlFloatLiteral          float_literal_val;
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
#define	IDENTIFIER	258
#define	ABSTRACT	259
#define	ANY	260
#define	ATTRIBUTE	261
#define	BOOLEAN	262
#define	CASE	263
#define	CHAR	264
#define	CONST	265
#define	CONTEXT	266
#define	CUSTOM	267
#define	DEFAULT	268
#define	DOUBLE	269
#define	ENUM	270
#define	EXCEPTION	271
#define	FACTORY	272
#define	FALSE_	273
#define	FIXED	274
#define	FLOAT	275
#define	IN	276
#define	INOUT	277
#define	INTERFACE	278
#define	LONG	279
#define	MODULE	280
#define	NATIVE	281
#define	OBJECT	282
#define	OCTET	283
#define	ONEWAY	284
#define	OUT	285
#define	PRIVATE	286
#define	PUBLIC	287
#define	RAISES	288
#define	READONLY	289
#define	SEQUENCE	290
#define	SHORT	291
#define	STRING	292
#define	STRUCT	293
#define	SUPPORTS	294
#define	SWITCH	295
#define	TRUE_	296
#define	TRUNCATABLE	297
#define	TYPEDEF	298
#define	UNION	299
#define	UNSIGNED	300
#define	VALUEBASE	301
#define	VALUETYPE	302
#define	VOID	303
#define	WCHAR	304
#define	WSTRING	305
#define	PRAGMA	306
#define	PRAGMA_PREFIX	307
#define	PRAGMA_ID	308
#define	PRAGMA_VERSION	309
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

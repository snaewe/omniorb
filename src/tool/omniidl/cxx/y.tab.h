typedef union {
  char*                    id_val;
  int                      int_val;
  IDL_ULong                ulong_val;
  IdlIntLiteral            int_literal_val;
#ifndef __VMS
  IdlFloatLiteral          float_literal_val;
#else
  double                   float_literal_val;
#endif
  char                     char_val;
  char*                    string_val;
  IDL_WChar                wchar_val;
  IDL_WChar*               wstring_val;
  IDL_Boolean              boolean_val;
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
#define	LOCAL	278
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
#define	OMNI_PRAGMA	310
#define	END_PRAGMA	311
#define	UNKNOWN_PRAGMA_BODY	312
#define	INTEGER_LITERAL	313
#define	CHARACTER_LITERAL	314
#define	WIDE_CHARACTER_LITERAL	315
#define	FLOATING_PT_LITERAL	316
#define	STRING_LITERAL	317
#define	WIDE_STRING_LITERAL	318
#define	FIXED_PT_LITERAL	319
#define	SCOPE_DELIM	320
#define	LEFT_SHIFT	321
#define	RIGHT_SHIFT	322


extern YYSTYPE yylval;

" Vim syntax file
" Language: saffire
" Maintainer: Joshua Thijssen <jthijssen@saffire-lang.org>
" Last Change:  Juni 9, 2013
" URL: http://github.com/saffire

" Heavily based on the php.vim syntax found at http://www.vim.org/scripts/script.php?script_id=1571

" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

if !exists("main_syntax")
  let main_syntax = 'saffire'
endif


syn case ignore

" Keywords
syn keyword sfKeyword while if else use as import from do for foreach switch class extends implements inherits
syn keyword sfKeyword abstract final interface const static public protected private method property
syn keyword sfKeyword catch finally throw return break breakelse continue try default goto case self parent yield

syn keyword sfClass string numerical regex boolean
syn keyword sfObject true false null

syn keyword sfTodo todo fixme xxx contained

" Comments
syn region sfDocBlock start="/\*\*" end="\*/" contains=sfTodo extend
syn region sfComment  start="/\*" end="\*/" extend
syn match  sfComment  "//.\{-}\(?>\|$\)\@="

syn region  sfStringSingle matchgroup=None start=+'+ skip=+\\\\\|\\'+ end=+'+  keepend extend
syn region  sfStringDouble matchgroup=None start=+"+ skip=+\\\\\|\\'+ end=+"+  keepend extend

syn match sfOperator "[-=+%^&|*!.~?:,]" display
syn match sfOperator "[-+*/%^&|.]="  display
" syn match sfOperator "in"  display
syn match sfOperator "/[^*/]"me=e-1  display


" Number
syn match sfNumber "-\=\<\d\+\>"  display
syn match sfNumber "\<0x\x\{1,8}\>" display

"syn region  sfParent matchgroup=Delimiter start="(" end=")"  transparent
"syn region  sfParent matchgroup=Delimiter start="\[" end="\]"  transparent
syn match sfParent "[({[\]})]" display


" ================================================================

let b:current_syntax = "saffire"

hi def link sfOperator Operator
hi def link sfParent Delimiter
hi def link sfNumber  Number
hi def link sfObject type
hi def link sfClass Type
hi def link sfStringSingle  String
hi def link sfStringDouble  String
hi def link sfStructure Structure
hi def link sfKeyword StorageClass
hi def link sfComment Comment
hi def link sfDocBlock NonText
hi def link sfTodo Todo


" Vim syntax file
" Language:	GNU Assembler
" Maintainer:	Erik Wognsen <erik.wognsen@gmail.com>
"		Previous maintainer:
"		Kevin Dahlhausen <kdahlhaus@yahoo.com>
" Last Change:	2014 Feb 04

" Thanks to Ori Avtalion for feedback on the comment markers!

" quit when a syntax file was already loaded
if exists("b:current_syntax")
  finish
endif

let s:cpo_save = &cpo
set cpo&vim

"syn case ignore

syn match blasmLabel		"[a-z_][a-z0-9_]*:"he=e-1
syn match blasmIdentifier		"[a-z_][a-z0-9_]*"

" Various #'s as defined by GAS ref manual sec 3.6.2.1
" Technically, the first decNumber def is actually octal,
" since the value of 0-7 octal is the same as 0-7 decimal,
" I (Kevin) prefer to map it as decimal:
syn match decNumber		"0\+[1-7]\=[\t\n$,; ]"
syn match decNumber		"[1-9]\d*"
syn match octNumber		"0[0-7][0-7]\+"
syn match hexNumber		"0[xX][0-9a-fA-F]\+"
syn match binNumber		"0[bB][0-1]*"

syn keyword blasmTodo		contained TODO

syn keyword nasmStdInstruction	nop push pop swap dup add sub mul div addf subf mulf divf jmp jif call ret native and or xor shr shl bot eq gt gef not read8 read16 read32 read64 write8 write16 write32 write64 halt print_debug 

" Line comment characters depend on the target architecture and command line
" options and some comments may double as logical line number directives or
" preprocessor commands. This situation is described at
" http://sourceware.org/binutils/docs-2.22/as/Comments.html
" Some line comment characters have other meanings for other targets. For
" example, .type directives may use the `@' character which is also an ARM
" comment marker.
" As a compromise to accommodate what I arbitrarily assume to be the most
" frequently used features of the most popular architectures (and also the
" non-GNU assembly languages that use this syntax file because their asm files
" are also named *.asm), the following are used as line comment characters:
syn match blasmComment		"[#;!|].*" contains=asmTodo

" Side effects of this include:
" - When `;' is used to separate statements on the same line (many targets
"   support this), all statements except the first get highlighted as
"   comments. As a remedy, remove `;' from the above.
" - ARM comments are not highlighted correctly. For ARM, uncomment the
"   following two lines and comment the one above.
"syn match asmComment		"@.*" contains=asmTodo
"syn match asmComment		"^#.*" contains=asmTodo

" Advanced users of specific architectures will probably want to change the
" comment highlighting or use a specific, more comprehensive syntax file.

syn match blasmInclude		"%include"

" Assembler directives start with a '.' and may contain upper case (e.g.,
" .ABORT), numbers (e.g., .p2align), dash (e.g., .app-file) and underscore in
" CFI directives (e.g., .cfi_startproc). This will also match labels starting
" with '.', including the GCC auto-generated '.L' labels.
syn match blasmDirective		"%[A-Za-z][0-9A-Za-z-_]*"

syn match	blasmSpecial	display contained "\\\(x\x\+\|\o\{1,3}\|.\|$\)"
syn match	blasmFormat		display "%\(\d\+\$\)\=[-+' #0*]*\(\d*\|\*\|\*\d\+\$\)\(\.\(\d*\|\*\|\*\d\+\$\)\)\=\([hlLjzt]\|ll\|hh\)\=\([aAbdiuoxXDOUfFeEgGcCsSpn]\|\[\^\=.[^]]*\]\)" contained
syn region	blasmString		start=+"+ end=+"+ contains=cSpecial,cFormat,@Spell extend

syn case match

" Define the default highlighting.
" Only when an item doesn't have highlighting yet

" The default methods for highlighting.  Can be overridden later
hi def link blasmLabel	Label
hi def link blasmComment	Comment
hi def link blasmTodo	Todo
hi def link blasmDirective	Statement

hi def link blasmInclude	Include

hi def link hexNumber	Number
hi def link decNumber	Number
hi def link octNumber	Number
hi def link binNumber	Number

hi def link blasmIdentifier	Identifier

hi def link blasmString		String

hi def link nasmStdInstruction	Statement

let b:current_syntax = "blasm"

let &cpo = s:cpo_save
unlet s:cpo_save

" vim: ts=8

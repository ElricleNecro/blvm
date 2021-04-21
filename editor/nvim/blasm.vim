" Vim syntax file
" Language: Basm

" Usage Instructions
" Put this file in .vim/syntax/basm.vim
" and add in your .vimrc file the next line:
" autocmd BufRead,BufNewFile *.basm set filetype=basm

if exists("b:current_syntax")
  finish
endif

syntax keyword basmTodos TODO XXX FIXME NOTE

" Language keywords
syntax keyword basmKeywords nop push pop dup print_debug
syntax keyword basmKeywords add sub mul div mod imul idiv modi
syntax keyword basmKeywords addf subf mulf divf
syntax keyword basmKeywords jmp jif halt swap not
syntax keyword basmKeywords ieq ige igt ile ilt ine
syntax keyword basmKeywords eq ge gt le lt ne
syntax keyword basmKeywords gef gtf lef ltf
syntax keyword basmKeywords ret call native
syntax keyword basmKeywords and or xor shr shl bnot
syntax keyword basmKeywords read8 read16 read32 read64
syntax keyword basmKeywords write8 write16 write32 write64
syntax keyword basmKeywords i2f u2f f2i f2u

" Comments
syntax region basmCommentLine start=";" end="$"   contains=basmTodos
syntax region basmDirective start="%" end=" "

" Numbers
syntax match basmDecInt display "\<[0-9][0-9_]*"
syntax match basmHexInt display "\<0[xX][0-9a-fA-F][0-9_a-fA-F]*"
syntax match basmFloat  display "\<[0-9][0-9_]*\%(\.[0-9][0-9_]*\)"
syntax match basmOctInt  display "0[0-7][0-7]\+"
syntax match basmHexInt  display "0[xX][0-9a-fA-F]\+"
syntax match basmBinInt  display "0[bB][0-1]*"

" Strings
syntax region basmString start=/\v"/ skip=/\v\\./ end=/\v"/
syntax region basmString start=/\v'/ skip=/\v\\./ end=/\v'/

" Set highlights
highlight default link basmTodos Todo
highlight default link basmKeywords Keyword
highlight default link basmCommentLine Comment
highlight default link basmDirective PreProc
highlight default link basmDecInt Number
highlight default link basmHexInt Number
highlight default link basmOctInt Number
highlight default link basmHexInt Number
highlight default link basmBinInt Number
highlight default link basmFloat Float
highlight default link basmString String

let b:current_syntax = "basm"

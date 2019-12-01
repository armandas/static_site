
" Encoding stuff
set encoding=utf-8
setglobal fileencoding=utf-8
set fileencodings=utf-8

" Search
set hlsearch
set incsearch
set ignorecase

" Quick save (Ctrl+S)
nmap <c-s> :w<CR>
vmap <c-s> <Esc><c-s>
imap <c-s> <Esc><c-s>

" Editing stuff
set autoindent
set number
syntax enable
colorscheme eclm_wombat "slate
set guifont=Inconsolata\ Medium\ 12
set tabstop=8
"set expandtab
"set et
set shiftwidth=8
set softtabstop=8

" No beeps
set vb
" No toolbar
set guioptions-=T

" Other stuff
set history=1000
set statusline=%<%F%h%m%r%=\[%B\]\ %l,%c%V\ %P
set laststatus=1

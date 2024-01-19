#limit descriptors unlimited
#limit coredumpsize 0

#--- Set environment
setenv LC_CTYPE en_US.UTF-8
setenv LANG en_US.UTF-8

setenv MALLOC_CHECK_ 0
setenv EDITOR  emacs		    #define emacs as default text editor
setenv LESS 'm c w i R'             #Options for less
unsetenv LESSOPEN
setenv MM_CHARSET iso-8859-1
unsetenv LESSCHARSET
setenv PAGER	 less		    #use "less" to read man pages

unset fignore
unset ignoreeof
set noclobber

#--- Set tcsh
if($?tcsh)then
  set prompt = "%B%m>%b"    #set command prompt
  set history=5000	#remember the last 100 commands
  set savehist=50000	#save 30 commands between logins
  set nobeep            #the shell is not supposed to beep at you
  set fignore=".o .dvi .aux .toc .bak .BAK .old "
  set autolist = ambiguous
endif


alias m          "less"
alias r          "nice root -l "
alias rr         "nice root -b -l "
alias rfsla      "find . -type d -print -exec sh -c 'fs setacl $0 \!* rlidwk' {} \;"
alias maek      'make'
alias cmaek     'cmake'
alias mm        'make -j10 '
alias m4        'make -j4 '
alias m4c       'make clean && make -j4 '
alias gmaek     'make'
alias gm        'gunzip -f -c \!* | less'
alias u         uptime
alias cd        'cd \!*'
alias pwd       'echo $cwd'
alias ls        'ls '
alias l         'ls -ltrG'
alias ll        'ls -al'
alias lm        'ls -rtl \!* |less'
alias purge     'rm -f \!*/*~ \!*/.*~ \!*/#*#  \!*/error___* \!*/dead.letter '
alias pu        'purge .;l'
alias grep      'grep -i'
alias t         'top -o cpu -s 5 '
alias tm        'top -o rsize -s 5 '
alias wo        'find . -name \*\!*\* -print'

# --- Typos
# ---------
alias mroe      less
alias maek      make
alias wpd       'echo $cwd'
alias pw        pwd
alias dpwd      pwd
alias wdp       pwd
alias wchih     which
alias whcih     which
alias rehahs    rehash


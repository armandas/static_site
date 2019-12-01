PCKGS = python3 \
tilda \
filezilla \
gnome-do \
vim-gnome \
htop \
unrar \
git-core \
octave3.2 \
vlc \
meld \
ttf-mscorefonts-installer \
ghex \
mplayer \
smplayer

FLAGS = -y --force-yes

all:
	@echo "Type: 'sudo make install' to install packages."

.PHONY: install
install:
	add-apt-repository ppa:ripps818/coreavc
	add-apt-repository ppa:rvm/smplayer
	apt-get update && apt-get install $(FLAGS) $(PCKGS)

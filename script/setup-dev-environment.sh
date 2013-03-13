#################################################################################################
# Setup pkgconfig path and Boitho home environment variables					#
#################################################################################################

# Remove and readd export PKG_CONFIG_PATH
perl -ni -le 'print unless /export PKG_CONFIG_PATH/' /home/boitho/.bash_profile
echo "export PKG_CONFIG_PATH=\$PKG_CONFIG_PATH:/usr/lib/pkgconfig/" >> /home/boitho/.bash_profile

# Remove and readd export BOITHOHOME
perl -ni -le 'print unless /export BOITHOHOME/' /home/boitho/.bash_profile
echo "export BOITHOHOME=/home/boitho/boithoTools" >> /home/boitho/.bash_profile


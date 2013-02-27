#################################################################################################
# Setup and import code from GitHub			    					#
#################################################################################################

git clone --no-checkout git://github.com/searchdaimon/enterprise-search.git /home/boitho/boithoTools.git
mv /home/boitho/boithoTools.git/.git /home/boitho/boithoTools/
cd /home/boitho/boithoTools/
# This wil download the code
git reset --hard HEAD

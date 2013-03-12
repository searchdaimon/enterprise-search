#################################################################################################
# Setting up ssh to allow logins from root with password.                                       #
#################################################################################################

SSHDCONFIG=/etc/ssh/sshd_config

# Permit root to log directly in
egrep "^PermitRootLogin no$" $SSHDCONFIG >/dev/null && sed -i.bak -e 's/^PermitRootLogin no$/PermitRootLogin yes/' $SSHDCONFIG

# Permit password logins
egrep "^PasswordAuthentication no$" $SSHDCONFIG >/dev/null && sed -i.bak -e 's/^PasswordAuthentication no$/PasswordAuthentication yes/' $SSHDCONFIG

# Restarting the ssh daimon
/etc/init.d/sshd restart


For testing purposes the pacman repository can be changed to ones managed by bxt by the following steps:

1. Backup current pacman.conf file:
   ```bash
   sudo cp /etc/pacman.conf /etc/pacman.conf.bak
   ```
   
2. Edit pacman configuration to point to the bxt-managed repo:
   ```ini
   [core]
   SigLevel = PackageRequired
   Server = https://<bxt-server-name>/box/unstable/core/x86_64/

   [extra]
   SigLevel = PackageRequired
   Server = https://<bxt-server-name>/box/unstable/extra/x86_64/

   [community]
   SigLevel = PackageRequired
   Server = https://<bxt-server-name>/box/unstable/community/x86_64/
    
   [multilib]
   SigLevel = PackageRequired
   Server = https://<bxt-server-name>/box/unstable/multilib/x86_64/
   ```

This will prevent pacman from using the mirrorlist in favor of bxt.

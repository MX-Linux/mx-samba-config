## About mx-samba-config

[![latest packaged version(s)](https://repology.org/badge/latest-versions/mx-samba-config.svg)](https://repology.org/project/mx-samba-config/versions)
[![build result](https://build.opensuse.org/projects/home:mx-packaging/packages/mx-samba-config/badge.svg?type=default)](https://software.opensuse.org//download.html?project=home%3Amx-packaging&package=mx-samba-config)

mx-samba-config is a GUI for managing Samba shares and users on MX Linux.

mx-samba-config is a Samba configuration tool designed to work with the same samba “usershare” system used by the thunar-shares-plugin and Dolphin file sharing capabilities.

It provides an interface that allows users to setup up Samba shares for folders they own, as well as adding and removing samba users.

## Troubleshooting share listing

MX Samba Config manages usershares, not share sections defined in `/etc/samba/smb.conf`.
Shares configured there can work even when listing usershares fails. `testparm` checks
the Samba configuration but does not establish that usershares can be read.

If the app reports "Error listing shares", open the error details or run the following
as the same normal user who runs the app:

```sh
net usershare info
id
testparm -s --parameter-name='usershare path'
testparm -s --parameter-name='usershare max shares'
```

The `net` error is needed to distinguish directory access problems, invalid usershare
files, and other Samba errors. Check the ownership and permissions of the configured
usershare directory (normally `/var/lib/samba/usershares`) and any files named in the
error. Membership in `sambashare` must be active in the current login session; log out
and back in after changing group membership. Avoid deleting shares or changing
permissions until the error identifies the problem.

Remote write access also requires the connecting Samba account to have write access
to the shared folder through its Unix permissions or filesystem ACLs. Selecting
"Full Access" in the app does not change those filesystem permissions.

## Help translate mx-samba-config in your language

Translation for mx-samba-config takes place on Transifex.

https://www.transifex.com/anticapitalista/signin/?next=/anticapitalista/antix-development/mx-samba-config/

![image](https://github.com/MX-Linux/mx-samba-config/assets/418436/fb96f996-0287-40f8-831e-ed6438fd2f24)

## AppImage

Download the x86-64 AppImage from a GitHub release’s **Assets**, make it executable with `chmod +x`, and launch it as your desktop user. The host supplies Samba (including `smbd`, `net`, `testparm` and the account tools) and its configured shares. Administrative actions reopen the AppImage with the host’s elevation tool and require working FUSE. Run it from a location that the administrator can access.

To build locally on Debian 13 (trixie), install the build dependencies and run the packaging script:

```sh
sudo apt-get install \
    ca-certificates git build-essential cmake ninja-build dpkg-dev curl file \
    qt6-base-dev qt6-tools-dev qt6-tools-dev-tools qt6-svg-plugins qt6-translations-l10n \
    qt6-wayland
./make-appimage.sh
```

The script writes `build-appimage/mx-samba-config-<version>-x86_64.AppImage` using the version in `debian/changelog`. GitHub Actions builds the same image and attaches it to each non-Arch release.

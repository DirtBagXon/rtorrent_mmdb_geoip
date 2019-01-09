# rtorrent_mmdb_geoip2
Adds Maxmind MMDB functionality to rtorrent client for enhanced detail on remote peer addresses.

#### For use with Free Maxmind GeoLite MMDB databases

Based upon an old **0.8.9** GeoIP2 patch (author unknown) and [russellsimpkins-nyt / geo_functions.c](https://gist.github.com/russellsimpkins-nyt/31f8ae504639e05e57a61e775a8cce6d)

As Maxmind's GeoIP _.dat_ is now obsolete, this replaces GeoIP lookup with new _.mmdb_ GeoLite City and ASNum databases.

Expected DB locations defined in '_geo_functions.h_':
```
#ifndef MAX_CITY_DB
#define MAX_CITY_DB "/usr/share/GeoIP/GeoLite2-City.mmdb"
#endif

#ifndef MAX_AS_DB
#define MAX_AS_DB "/usr/share/GeoIP/GeoLite2-ASN.mmdb"
#endif
```
![peer_list](https://raw.githubusercontent.com/DirtBagXon/rtorrent_mmdb_geoip/master/peer_list.png)
![peer_info](https://raw.githubusercontent.com/DirtBagXon/rtorrent_mmdb_geoip/master/peer_info.png)

**Note:** This is not a patch for the latest rtorrent **0.9.7** client, however it appears an easy port when applied to latest tarball...

**Note:** This patch contains a 0.8.9 '_range_map.h_' backport patch, which is required for this changeset. 
- '_range_map.h_' will fail to merge on later branches.

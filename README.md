# rtorrent_mmdb_geoip2
Adds Maxmind MMDB functionality to rtorrent client for enhanced detail on remote peer addresses.

For use with **Free** Maxmind GeoLite MMDB databases.

**UPDATE FROM DECEMBER 30, 2019:** You are now required to create a **free account** to download databases.

https://www.maxmind.com/en/geolite2/signup

GeoLite2 Downloadable Databases: https://dev.maxmind.com/geoip/geoip2/geolite2/#Downloads

Update via: https://dev.maxmind.com/geoip/geoipupdate/

This patch was based upon an old **0.8.9** GeoIP patch (author unknown) and [russellsimpkins-nyt / geo_functions.c](https://gist.github.com/russellsimpkins-nyt/31f8ae504639e05e57a61e775a8cce6d)

**Note:** Patches for **0.8.9**, **0.9.6** and **0.9.8** are provided.

As Maxmind's GeoIP _.dat_ is now obsolete, this replaces GeoIP lookup with new _.mmdb_ GeoLite City and ASNum databases.

Additional libraries required for build:

    apt-get install libmaxminddb-dev libmaxminddb0 geoipupdate


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

**Note:** The 0.8.9 patch contains a '_range_map.h_' backport patch, which is required for this changeset. 
- '_range_map.h_' will fail to merge where this patch has already been applied.

# Premium / VIP for OrsCore

Module: **`modules/mod-premium`** (no core patches).

## Features

- Account VIP in auth DB (`account_premium`)
- Bonus balance in `account_donate.bonuses`
- Shop: free 7-day once (`.vip free1day`), buy 1/7/31 days (`.vip buy`)
- VIP commands: buff, bank, mail, repair, taxi, home, capital, app/summon, textcolor, …
- GM: `.vip set|del|addbonus`
- Client panel: `addon/Premium` (`/premium` / `/vip`) — opens **without** VIP

## Install (server)

1. Rebuild worldserver with modules.
2. `conf/premium.conf.dist` → `premium.conf`, `Premium.Enable = 1`.
3. Apply `data/sql/db-auth/base/premium_auth.sql` and `data/sql/db-world/base/premium_strings.sql`.

## Install (client)

Copy `addon/Premium` → `Interface/AddOns/Premium`, restart client.

## Prices (conf)

```
Premium.Price.1Day = 10
Premium.Price.7Days = 350
Premium.Price.31Days = 600
```

GM grant bonuses: `.vip addbonus 1000`

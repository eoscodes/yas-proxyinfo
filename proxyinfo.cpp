#include <eosiolib/eosio.hpp>

class proxyinfo_contract : public eosio::contract {
    public:
        proxyinfo_contract(account_name self)
        :eosio::contract(self),
        proxies(_self, _self),
        proxiestmp(_self, _self)
        {}

        /**
         *  Set proxy info
         *
         *  @param owner - the proxy account name
         *  @param name - human readable name
         *  @param slogan - a short description
         *  @param philosophy - voting philosophy
         *  @param background - optional. who is the proxy?
         *  @param website - optional. url to website
         *  @param logo - optional. url to an image with the size of 256 x 256 px
         *  @param contact - optional. contact 
         */
        void set(const account_name owner, std::string name, std::string slogan, std::string philosophy, std::string background, std::string website, std::string logo, std::string contact) {
            // Validate input
            eosio_assert(!name.empty(), "name required");
            eosio_assert(name.length() <= 64, "name too long");

            eosio_assert(slogan.length() <= 64, "slogan too long");

            eosio_assert(philosophy.length() <= 1024, "philosophy too long");

            eosio_assert(background.length() <= 1024, "background too long");

            eosio_assert(website.length() <= 256, "website too long");
            if (!website.empty()) {
                eosio_assert(website.substr(0, 4) == "http", "website should begin with http");
            }

            eosio_assert(logo.length() <= 256, "logo too long");
            if (!logo.empty()) {
                eosio_assert(logo.substr(0, 4) == "http", "logo should begin with http");
            }

            eosio_assert(contact.length() <= 64, "telegram too long");

            // Require auth from the proxy account
            require_auth(owner);

            // Check if exists
            auto current = proxies.find(owner);

            // Update
            if (current != proxies.end()) {
                proxies.modify(current, owner, [&]( auto& i ) {
                    i.owner = owner;
                    i.name = name;
                    i.website = website;
                    i.slogan = slogan;
                    i.philosophy = philosophy;
                    i.background = background;
                    i.logo = logo;
                    i.contact = contact;
                });

            // Insert
            } else {
                proxies.emplace(owner, [&]( auto& i ) {
                    i.owner = owner;
                    i.name = name;
                    i.website = website;
                    i.slogan = slogan;
                    i.philosophy = philosophy;
                    i.background = background;
                    i.logo = logo;
                    i.contact = contact;
                });
            }
        }

        /**
         * Remove all proxy info.
         *
         * @param proxy - the proxy account name.
         */
        void remove(const account_name owner) {
            // Require auth from the proxy account
            require_auth(owner);

            // Delete record
            auto lookup = proxies.find(owner);
            if (lookup != proxies.end()) {
                proxies.erase(lookup);
            }
        }

        /**
         * Migrate data from "proxiestmp" to "proxies", for when we are changing schema.
         */
        void migratetmp() {
            require_auth(_self);
            for (const auto & row : proxiestmp) {
                //eosio::print("Migrating owner=", row.owner, ", name=", row.name, ", website=", row.website, "\n");
                proxies.emplace(_self, [&](auto & i) {
                    i.owner = row.owner;
                    i.name = row.name;
                    i.website = row.website;
                });
            }
        }

        /**
         * Erase all data in "proxiestmp", used when we are changing schema.
         */
        void erasetmp() {
            require_auth(_self);
            for(auto itr = proxiestmp.begin(); itr != proxiestmp.end();) {
                itr = proxiestmp.erase(itr);
            }
        }

    private:
        // @abi table proxies i64
        struct proxy {
            account_name owner;
            std::string name;
            std::string website;
            std::string slogan;
            std::string philosophy;
            std::string background;
            std::string logo;
            std::string contact;

            auto primary_key()const { return owner; }
            EOSLIB_SERIALIZE(proxy, (owner)(name)(website)(slogan)(philosophy)(background)(logo)(contact))
        };

        typedef eosio::multi_index<N(proxies), proxy> proxy_table;
        proxy_table proxies;

        // @abi table proxiestmp i64
        struct proxytmp {
            account_name owner;
            std::string name;
            std::string website;

            auto primary_key()const { return owner; }
            EOSLIB_SERIALIZE(proxytmp, (owner)(name)(website))
        };

        typedef eosio::multi_index<N(proxiestmp), proxytmp> proxytmp_table;
        proxytmp_table proxiestmp;
};

EOSIO_ABI(proxyinfo_contract, (set)(remove)(migratetmp)(erasetmp))

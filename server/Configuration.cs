﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Glove.INI;

namespace SockScape {
    public static class Configuration {
        private static SettingsFile Settings;

        static Configuration() {
            Settings = new SettingsFile(
                "config.ini",
                new List<SectionRules> {
                    new SectionRules {
                        Name = "General",
                        Required = true,
                        RequiredFields = new string[] {
                            "Run Master",
                            "Master Port",
                            "Master Addr",
                            "Max Users"
                        }
                    },

                    new SectionRules {
                        Name = "Database",
                        Required = true,
                        RequiredFields = new string[] {
                            "Server",
                            "Username",
                            "Password",
                            "Database"
                        }
                    },

                    new SectionRules {
                        Name = "Server",
                        AllowMultiple = true,
                        Required = true,
                        RequiredFields = new string[] {
                            "Id",
                            "Port"
                        }
                    }
                }
            );
        }

        public static Section Get(string section) {
            return Settings[section];
        }

        public static Instance General {
            get {
                return Settings["General"][0];
            }
        }

        public static Instance Database {
            get {
                return Settings["Database"][0];
            }
        }

        public static Section Servers {
            get {
                return Settings["Server"];
            }
        }
    }
}

CREATE TABLE version
(
   version  INTEGER,
   created  TEXT
);
INSERT INTO version (version, created) VALUES (1, strftime('%Y-%m-%d %H:%M:%S', 'now'));

CREATE TABLE mfiles
(
   filename    TEXT NOT NULL UNIQUE,
   artist      TEXT,
   album       TEXT,
   title       TEXT,
   comment     TEXT,
   genre       TEXT,
   length      INTEGER,
   bitrate     INTEGER,
   samplerate  INTEGER,
   channels    INTEGER,
   last_update TEXT,
   PRIMARY KEY (filename)
);
CREATE INDEX mfiles_artist  ON mfiles (artist);
CREATE INDEX mfiles_album   ON mfiles (album);
CREATE INDEX mfiles_title   ON mfiles (title);
CREATE INDEX mfiles_comment ON mfiles (comment);
CREATE INDEX mfiles_genre   ON mfiles (genre);

CREATE TABLE playlists
(
   name           TEXT     NOT NULL,
   display_order  INTEGER  NOT NULL UNIQUE,
   PRIMARY KEY (name)
);

CREATE TABLE playlist_contents
(
   playlist       TEXT,
   display_order  INTEGER,
   mfile          TEXT,
   PRIMARY KEY (playlist, display_order, mfile),
   FOREIGN KEY (playlist)  REFERENCES playlists(name),
   FOREIGN KEY (mfile)     REFERENCES mfiles(filename)
);
CREATE INDEX playlists_content ON playlist_contents (playlist, display_order, mfile);

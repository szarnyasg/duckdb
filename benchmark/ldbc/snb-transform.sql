-- basic
INSERT INTO Basic_Organisation                   SELECT id, type, name, url                                                              FROM Organisation;
INSERT INTO Basic_Organisation_isLocatedIn_Place SELECT id, isLocatedIn_Place                                                            FROM Organisation;

INSERT INTO Basic_Place                          SELECT id, name, url, type                                                              FROM Place;
INSERT INTO Basic_Place_isPartOf_Place           SELECT id, isPartOf_Place                                                               FROM Place;

INSERT INTO Basic_TagClass                       SELECT id, name, url                                                                    FROM TagClass;
INSERT INTO Basic_TagClass_hasType_TagClass      SELECT id, hasType_TagClass                                                             FROM TagClass;

INSERT INTO Basic_Tag                            SELECT id, name, url                                                                    FROM Tag;
INSERT INTO Basic_Tag_isSubclassOf_TagClass      SELECT id, isSubclassOf_TagClass                                                        FROM Tag;

INSERT INTO Basic_Comment                        SELECT creationDate, id, locationIP, browserUsed, content, length                       FROM Comment;
INSERT INTO Basic_Comment_hasCreator_Person      SELECT creationDate, id, hasCreator_Person                                              FROM Comment;
INSERT INTO Basic_Comment_isLocatedIn_Place      SELECT creationDate, id, isLocatedIn_Place                                              FROM Comment;
INSERT INTO Basic_Comment_replyOf_Post           SELECT creationDate, id, replyOf_Post                                                   FROM Comment;
INSERT INTO Basic_Comment_replyOf_Comment        SELECT creationDate, id, replyOf_Comment                                                FROM Comment;

INSERT INTO Basic_Forum                          SELECT creationDate, id, title                                                          FROM Forum;
INSERT INTO Basic_Forum_hasModerator_Person      SELECT creationDate, id, hasModerator_Person                                            FROM Forum;

INSERT INTO Basic_Post                           SELECT creationDate, id, imageFile, locationIP, browserUsed, language, content, length  FROM Post;
INSERT INTO Basic_Post_hasCreator_Person         SELECT creationDate, id, hasCreator_Person                                              FROM Post;
INSERT INTO Basic_Post_Forum_containerOf         SELECT creationDate, id, Forum_containerOf                                              FROM Post;
INSERT INTO Basic_Post_isLocatedIn_Place         SELECT creationDate, id, isLocatedIn_Place                                              FROM Post;

INSERT INTO Basic_Person                         SELECT creationDate, id, firstName, lastName, gender, birthday, locationIP, browserUsed FROM Person;
INSERT INTO Basic_Person_isLocatedIn_Place       SELECT creationDate, id, isLocatedIn_Place                                              FROM Person;
INSERT INTO Basic_Person_speaks                  SELECT creationDate, id, speaks                                                         FROM Person;
INSERT INTO Basic_Person_email                   SELECT creationDate, id, email                                                          FROM Person;

-- merge-foreign
INSERT INTO MergeForeign_Organisation  SELECT id, type, name, url, isLocatedIn_Place                                                                                                   FROM Organisation;
INSERT INTO MergeForeign_Place         SELECT id, name, url, type, isPartOf_Place                                                                                                      FROM Place;
INSERT INTO MergeForeign_TagClass      SELECT id, name, url, hasType_TagClass                                                                                                          FROM TagClass;
INSERT INTO MergeForeign_Tag           SELECT id, name, url, isSubclassOf_TagClass                                                                                                     FROM Tag;
INSERT INTO MergeForeign_Comment       SELECT creationDate, id, locationIP, browserUsed, content, length, hasCreator_Person, isLocatedIn_Place, replyOf_Post, replyOf_Comment          FROM Comment;
INSERT INTO MergeForeign_Forum         SELECT creationDate, id, title, hasModerator_Person                                                                                             FROM Forum;
INSERT INTO MergeForeign_Person        SELECT creationDate, id, firstName, lastName, gender, birthday, locationIP, browserUsed, isLocatedIn_Place, speaks, email                       FROM Person;
INSERT INTO MergeForeign_Post          SELECT creationDate, id, imageFile, locationIP, browserUsed, language, content, length, hasCreator_Person, Forum_containerOf, isLocatedIn_Place FROM Post;

-- composite
INSERT INTO Composite_Person SELECT creationDate, id, firstName, lastName, gender, birthday, locationIP, browserUsed, speaks, email FROM Person;

-- composite-merge-foreign
INSERT INTO Composite_MergeForeign_Person SELECT creationDate, id, firstName, lastName, gender, birthday, locationIP, browserUsed, isLocatedIn_Place, speaks, email FROM Person;

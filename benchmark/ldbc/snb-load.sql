-- static tables

COPY organisation FROM 'PATHVAR/static/organisationPOSTFIX' (DELIMITER '|', HEADER);
COPY place        FROM 'PATHVAR/static/placePOSTFIX'        (DELIMITER '|', HEADER);
COPY tag          FROM 'PATHVAR/static/tagPOSTFIX'          (DELIMITER '|', HEADER);
COPY tagclass     FROM 'PATHVAR/static/tagclassPOSTFIX'     (DELIMITER '|', HEADER);

-- dynamic tables

COPY comment                   FROM 'PATHVAR/dynamic/commentPOSTFIX'                     (DELIMITER '|', HEADER, TIMESTAMPFORMAT '%Y-%m-%dT%H:%M:%S.%g+00:00');
COPY comment_hasTag_tag        FROM 'PATHVAR/dynamic/comment_hasTag_tagPOSTFIX'          (DELIMITER '|', HEADER, TIMESTAMPFORMAT '%Y-%m-%dT%H:%M:%S.%g+00:00');

COPY post                      FROM 'PATHVAR/dynamic/postPOSTFIX'                        (DELIMITER '|', HEADER, TIMESTAMPFORMAT '%Y-%m-%dT%H:%M:%S.%g+00:00');
COPY post_hasTag_tag           FROM 'PATHVAR/dynamic/post_hasTag_tagPOSTFIX'             (DELIMITER '|', HEADER, TIMESTAMPFORMAT '%Y-%m-%dT%H:%M:%S.%g+00:00');

COPY forum                     FROM 'PATHVAR/dynamic/forumPOSTFIX'                       (DELIMITER '|', HEADER, TIMESTAMPFORMAT '%Y-%m-%dT%H:%M:%S.%g+00:00');
COPY forum_hasMember_person    FROM 'PATHVAR/dynamic/forum_hasMember_personPOSTFIX'      (DELIMITER '|', HEADER, TIMESTAMPFORMAT '%Y-%m-%dT%H:%M:%S.%g+00:00');
COPY forum_hasTag_tag          FROM 'PATHVAR/dynamic/forum_hasTag_tagPOSTFIX'            (DELIMITER '|', HEADER, TIMESTAMPFORMAT '%Y-%m-%dT%H:%M:%S.%g+00:00');

COPY person                    FROM 'PATHVAR/dynamic/personPOSTFIX'                      (DELIMITER '|', HEADER, TIMESTAMPFORMAT '%Y-%m-%dT%H:%M:%S.%g+00:00');
COPY person_hasInterest_tag    FROM 'PATHVAR/dynamic/person_hasInterest_tagPOSTFIX'      (DELIMITER '|', HEADER, TIMESTAMPFORMAT '%Y-%m-%dT%H:%M:%S.%g+00:00');
COPY person_studyAt_university FROM 'PATHVAR/dynamic/person_studyAt_organisationPOSTFIX' (DELIMITER '|', HEADER, TIMESTAMPFORMAT '%Y-%m-%dT%H:%M:%S.%g+00:00');
COPY person_workAt_company     FROM 'PATHVAR/dynamic/person_workAt_organisationPOSTFIX'  (DELIMITER '|', HEADER, TIMESTAMPFORMAT '%Y-%m-%dT%H:%M:%S.%g+00:00');
COPY person_likes_post         FROM 'PATHVAR/dynamic/person_likes_postPOSTFIX'           (DELIMITER '|', HEADER, TIMESTAMPFORMAT '%Y-%m-%dT%H:%M:%S.%g+00:00');
COPY person_likes_comment      FROM 'PATHVAR/dynamic/person_likes_commentPOSTFIX'        (DELIMITER '|', HEADER, TIMESTAMPFORMAT '%Y-%m-%dT%H:%M:%S.%g+00:00');

COPY person_knows_person ( creationDate, deletionDate, explicitlyDeleted, person1id, person2id) FROM 'PATHVAR/dynamic/person_knows_personPOSTFIX' (DELIMITER '|', HEADER, TIMESTAMPFORMAT '%Y-%m-%dT%H:%M:%S.%g+00:00');
COPY person_knows_person ( creationDate, deletionDate, explicitlyDeleted, person2id, person1id) FROM 'PATHVAR/dynamic/person_knows_personPOSTFIX' (DELIMITER '|', HEADER, TIMESTAMPFORMAT '%Y-%m-%dT%H:%M:%S.%g+00:00');


--select creationDate, deletionDate, id, firstName, lastName, gender, birthday, locationIP, browserUsed, isLocatedIn_Place into merge_foreign_person from person;
insert into merge_foreign_person  select creationDate, deletionDate, id, firstName, lastName, gender, birthday, locationIP, browserUsed, isLocatedIn_Place from person;

create view merge_foreign_person_email  as select id, unnest(string_split_regex(email,  ';')) as email  from person;
create view merge_foreign_person_speaks as select id, unnest(string_split_regex(speaks, ';')) as speaks from person;

EXPORT DATABASE 'csv-merge-foreign' (FORMAT CSV, DELIMITER '|');

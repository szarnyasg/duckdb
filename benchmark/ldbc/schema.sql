-- static tables

create table organisation (
    id bigint not null,
    type varchar(12) not null,
    name varchar(256) not null,
    url varchar(256) not null,
    isLocatedIn_Place bigint
);

create table place (
    id bigint not null,
    name varchar(256) not null,
    url varchar(256) not null,
    type varchar(12) not null,
    isPartOf_Place bigint
);

create table tagclass (
    id bigint not null,
    name varchar(256) not null,
    url varchar(256) not null,
    hasType_TagClass bigint
);

create table tag (
    id bigint not null,
    name varchar(256) not null,
    url varchar(256) not null,
    isSubclassOf_TagClass bigint not null
);

-- dynamic tables

create table comment (
    creationDate timestamp without time zone not null,
    deletionDate timestamp without time zone not null,
    explicitlyDeleted boolean not null,
    id bigint not null,
    locationIP varchar(40) not null,
    browserUsed varchar(40) not null,
    content varchar(2000) not null,
    length int not null,
    hasCreator_Person bigint not null,
    isLocatedIn_Place bigint not null,
    replyOf_Post bigint,
    replyOf_Comment bigint
);

create table comment_hasTag_tag (
    creationDate timestamp without time zone not null,
    deletionDate timestamp without time zone not null,
    id bigint not null,
    hasTag_Tag bigint not null
);

create table forum (
    creationDate timestamp without time zone not null,
    deletionDate timestamp without time zone not null,
    explicitlyDeleted boolean not null,
    id bigint not null,
    title varchar(256) not null,
    hasModerator_Person bigint not null
);

create table forum_hasMember_person (
    creationDate timestamp without time zone not null,
    deletionDate timestamp without time zone not null,
    explicitlyDeleted boolean not null,
    id bigint not null,
    hasMember_Person bigint not null
);

create table forum_hasTag_tag (
    creationDate timestamp without time zone not null,
    deletionDate timestamp without time zone not null,
    id bigint not null,
    hasTag_Tag bigint not null
);

create table person (
    creationDate timestamp without time zone not null,
    deletionDate timestamp without time zone not null,
    id bigint not null,
    firstName varchar(40) not null,
    lastName varchar(40) not null,
    gender varchar(40) not null,
    birthday date not null,
    locationIP varchar(40) not null,
    browserUsed varchar(40) not null,
    isLocatedIn_Place bigint not null,
    speaks varchar(640) not null,
    email varchar(8192) not null
);

create table person_hasInterest_tag (
    creationDate timestamp without time zone not null,
    deletionDate timestamp without time zone not null,
    id bigint not null,
    hasInterest_Tag bigint not null
);

create table person_knows_person (
    creationDate timestamp without time zone not null,
    deletionDate timestamp without time zone not null,
    explicitlyDeleted boolean not null,
    person1id bigint not null,
    person2id bigint not null
);

create table person_likes_comment (
    creationDate timestamp without time zone not null,
    deletionDate timestamp without time zone not null,
    explicitlyDeleted boolean not null,
    id bigint not null,
    likes_Comment bigint not null
);

create table person_likes_post (
    creationDate timestamp without time zone not null,
    deletionDate timestamp without time zone not null,
    explicitlyDeleted boolean not null,
    id bigint not null,
    likes_Post bigint not null
);

create table person_studyAt_university (
    creationDate timestamp without time zone not null,
    deletionDate timestamp without time zone not null,
    id bigint not null,
    studyAt_University bigint not null,
    classYear int not null
);

create table person_workAt_company (
    creationDate timestamp without time zone not null,
    deletionDate timestamp without time zone not null,
    id bigint not null,
    workAt_Company bigint not null,
    workFrom int not null
);

create table post (
    creationDate timestamp without time zone not null,
    deletionDate timestamp without time zone not null,
    explicitlyDeleted boolean not null,
    id bigint not null,
    imageFile varchar(40),
    locationIP varchar(40) not null,
    browserUsed varchar(40) not null,
    language varchar(40),
    content varchar(2000),
    length int not null,
    hasCreator_Person bigint not null,
    Forum_containerOf bigint not null,
    isLocatedIn_Place bigint not null
);

create table post_hasTag_tag (
    creationDate timestamp without time zone not null,
    deletionDate timestamp without time zone not null,
    id bigint not null,
    hasTag_Tag bigint not null
);

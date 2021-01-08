-- static tables

create table NonMergeForeign_Organisation (
    id bigint not null,
    type varchar(12) not null,
    name varchar(256) not null,
    url varchar(256) not null
);
create table NonMergeForeign_Organisation_isLocatedIn_Place (
    id bigint not null,
    isLocatedIn_Place bigint
);

create table NonMergeForeign_Place (
    id bigint not null,
    name varchar(256) not null,
    url varchar(256) not null,
    type varchar(12) not null
);
create table NonMergeForeign_Place_isPartOf_Place (
    id bigint not null,
    isPartOf_Place bigint
);

create table NonMergeForeign_TagClass (
    id bigint not null,
    name varchar(256) not null,
    url varchar(256) not null
);
create table NonMergeForeign_TagClass_hasType_TagClass (
    id bigint not null,
    hasType_TagClass bigint
);

create table NonMergeForeign_Tag (
    id bigint not null,
    name varchar(256) not null,
    url varchar(256) not null
);
create table NonMergeForeign_Tag_isSubclassOf_TagClass (
    id bigint not null,
    isSubclassOf_TagClass bigint not null
);

-- dynamic tables

create table NonMergeForeign_Comment (
    creationDate timestamp without time zone not null,
    id bigint not null,
    locationIP varchar(40) not null,
    browserUsed varchar(40) not null,
    content varchar(2000) not null,
    length int not null
);
create table NonMergeForeign_Comment_hasCreator_Person (
    creationDate timestamp without time zone not null,
    id bigint not null,
    hasCreator_Person bigint not null
);
create table NonMergeForeign_Comment_isLocatedIn_Place (
    creationDate timestamp without time zone not null,
    id bigint not null,
    isLocatedIn_Place bigint not null
);
create table NonMergeForeign_Comment_replyOf_Post (
    creationDate timestamp without time zone not null,
    id bigint not null,
    replyOf_Post bigint
);
create table NonMergeForeign_Comment_replyOf_Comment (
    creationDate timestamp without time zone not null,
    id bigint not null,
    replyOf_Comment bigint
);

create table NonMergeForeign_Forum (
    creationDate timestamp without time zone not null,
    id bigint not null,
    title varchar(256) not null
);
create table NonMergeForeign_Forum_hasModerator_Person (
    creationDate timestamp without time zone not null,
    id bigint not null,
    hasModerator_Person bigint not null
);

create table NonMergeForeign_Person (
    creationDate timestamp without time zone not null,
    id bigint not null,
    firstName varchar(40) not null,
    lastName varchar(40) not null,
    gender varchar(40) not null,
    birthday date not null,
    locationIP varchar(40) not null,
    browserUsed varchar(40) not null,
    speaks varchar(640) not null,
    email varchar(8192) not null
);
create table NonMergeForeign_Person_isLocatedIn_Place (
    creationDate timestamp without time zone not null,
    id bigint not null,
    isLocatedIn_Place bigint not null
);

create table NonMergeForeign_Post (
    creationDate timestamp without time zone not null,
    id bigint not null,
    imageFile varchar(40),
    locationIP varchar(40) not null,
    browserUsed varchar(40) not null,
    language varchar(40),
    content varchar(2000),
    length int not null
);
create table NonMergeForeign_Post_hasCreator_Person (
    creationDate timestamp without time zone not null,
    id bigint not null,
    hasCreator_Person bigint not null
);
create table NonMergeForeign_Post_Forum_containerOf (
    creationDate timestamp without time zone not null,
    id bigint not null,
    Forum_containerOf bigint not null
);
create table NonMergeForeign_Post_isLocatedIn_Place (
    creationDate timestamp without time zone not null,
    id bigint not null,
    isLocatedIn_Place bigint not null
);

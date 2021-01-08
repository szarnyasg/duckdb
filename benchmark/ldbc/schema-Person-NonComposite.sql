create table NonComposite_Person_speaks (
    creationDate timestamp without time zone not null,
    id bigint not null,
    speaks varchar(40) not null
);

create table NonComposite_Person_email (
    creationDate timestamp without time zone not null,
    id bigint not null,
    email varchar(256) not null
);

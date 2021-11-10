WITH RECURSIVE search_graph(link, level, path) AS (
        SELECT cast(1367 as int64), 0, cast(1367 as string) -- ARRAY[:person1Id::bigint]::bigint[]
      UNION ALL
          (WITH sg(link, level) as (select * from search_graph) -- Note: sg is only the diff produced in the previous iteration
          SELECT distinct k_person2id, x.level+1, case when path = '' then cast(k_person2id as string) else concat(path, ';', k_person2id) end
          FROM knows, sg x
          WHERE 1=1
          and x.link = k_person1id
          and not regexp_matches(path, concat('[^0-9]', k_person2id, '[^0-9]')) -- 'k_person2id <> ALL (path)' expressed with a regex
          -- stop if we have reached person2 in the previous iteration
          and not exists(select * from sg y where y.link = 21990232556256)
          -- skip reaching persons reached in the previous iteration
          and not exists(select * from sg y where y.link = k_person2id)
        )
)
select max(level) AS shortestPathLength from (
select level from search_graph where link = 21990232556256
union select -1) tmp;

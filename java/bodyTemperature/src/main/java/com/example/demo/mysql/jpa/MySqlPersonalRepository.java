package com.example.demo.mysql.jpa;

import java.util.List;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

@Repository
public interface MySqlPersonalRepository extends JpaRepository<PersonalDataEntity, Long> {
	@Query(value = "select * from personal where " + "name = :name", nativeQuery = true)
	PersonalDataEntity getByName(@Param("name") String name);
	@Query(value = "select * from personal where " + "id = :id", nativeQuery = true)
	PersonalDataEntity get(@Param("id") long id);
	@Query(value = "select * from personal where " + "mail = :mail", nativeQuery = true)
	PersonalDataEntity get(@Param("mail") String mail);
	List<PersonalDataEntity> findAll();
}

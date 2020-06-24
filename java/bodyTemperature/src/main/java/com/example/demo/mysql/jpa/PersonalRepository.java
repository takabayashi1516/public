package com.example.demo.mysql.jpa;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.JpaSpecificationExecutor;
import org.springframework.stereotype.Repository;

@Repository
public interface PersonalRepository extends JpaRepository<PersonalDataEntity, Long>,
		JpaSpecificationExecutor<PersonalDataEntity> {

//	@Query(value = "select * from personal where " + "name = :name", nativeQuery = true)
//	PersonalDataEntity getByName(@Param("name") String name);

//	@Query(value = "select * from personal where " + "id = :id", nativeQuery = true)
//	PersonalDataEntity get(@Param("id") long id);

//	@Query(value = "select * from personal where " + "mail = :mail", nativeQuery = true)
//	PersonalDataEntity get(@Param("mail") String mail);

//	List<PersonalDataEntity> findAll();
}

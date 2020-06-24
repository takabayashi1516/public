package com.example.demo.mysql.jpa;

import javax.persistence.criteria.CriteriaBuilder;
import javax.persistence.criteria.CriteriaQuery;
import javax.persistence.criteria.Predicate;
import javax.persistence.criteria.Root;

import org.springframework.data.jpa.domain.Specification;

public class PersonalSpec {

	@SuppressWarnings("serial")
	public static Specification<PersonalDataEntity> fieldsEquals(String field, Object value) {
		return (value == null)? null : new Specification<PersonalDataEntity>() {
			@Override
			public Predicate toPredicate(Root<PersonalDataEntity> root, CriteriaQuery<?> query,
					CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.equal(root.get(field), value);
			}
		};
	}

	@SuppressWarnings("serial")
	public static Specification<PersonalDataEntity> fieldNumberGreaterThans(String field, Long value) {
		return (value == null)? null : new Specification<PersonalDataEntity>() {
			@Override
			public Predicate toPredicate(Root<PersonalDataEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.greaterThan(root.get(field), value);
			}
		};
	}

	@SuppressWarnings("serial")
	public static Specification<PersonalDataEntity> fieldFloatGreaterThans(String field, Float value) {
		return (value == null)? null : new Specification<PersonalDataEntity>() {
			@Override
			public Predicate toPredicate(Root<PersonalDataEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.greaterThan(root.get(field), value);
			}
		};
	}

	@SuppressWarnings("serial")
	public static Specification<PersonalDataEntity> fieldNumberLessThans(String field, Long value) {
		return (value == null)? null : new Specification<PersonalDataEntity>() {
			@Override
			public Predicate toPredicate(Root<PersonalDataEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.lessThan(root.get(field), value);
			}
		};
	}

	@SuppressWarnings("serial")
	public static Specification<PersonalDataEntity> fieldFloatLessThans(String field, Float value) {
		return (value == null)? null : new Specification<PersonalDataEntity>() {
			@Override
			public Predicate toPredicate(Root<PersonalDataEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.lessThan(root.get(field), value);
			}
		};
	}


	@SuppressWarnings("serial")
	public static Specification<PersonalDataEntity> fieldNumberGreaterThanOrEquals(String field, Long value) {
		return (value == null)? null : new Specification<PersonalDataEntity>() {
			@Override
			public Predicate toPredicate(Root<PersonalDataEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.greaterThanOrEqualTo(root.get(field), value);
			}
		};
	}

	@SuppressWarnings("serial")
	public static Specification<PersonalDataEntity> fieldFloatGreaterThanOrEquals(String field, Float value) {
		return (value == null)? null : new Specification<PersonalDataEntity>() {
			@Override
			public Predicate toPredicate(Root<PersonalDataEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.greaterThanOrEqualTo(root.get(field), value);
			}
		};
	}

	@SuppressWarnings("serial")
	public static Specification<PersonalDataEntity> fieldNumberLessThanOrEquals(String field, Long value) {
		return (value == null)? null : new Specification<PersonalDataEntity>() {
			@Override
			public Predicate toPredicate(Root<PersonalDataEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.lessThanOrEqualTo(root.get(field), value);
			}
		};
	}

	@SuppressWarnings("serial")
	public static Specification<PersonalDataEntity> fieldFloatLessThanOrEquals(String field, Float value) {
		return (value == null)? null : new Specification<PersonalDataEntity>() {
			@Override
			public Predicate toPredicate(Root<PersonalDataEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.lessThanOrEqualTo(root.get(field), value);
			}
		};
	}
}

/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import Select, {
    GroupBase,
    SelectComponentsConfig,
    StylesConfig,
    components
} from "react-select";
import {
    branches,
    reposForBranch,
    architecturesForBranchAndRepo
} from "../utils/SectionUtils";
import { HTMLAttributes, useCallback, useEffect } from "react";
import {
    faCodeBranch,
    faCubes,
    faMicrochip
} from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import * as daisyui from "react-daisyui";
import { IconProp } from "@fortawesome/fontawesome-svg-core";
import { ClassNamesArg } from "@emotion/react";

type ISectionSelectorProps = HTMLAttributes<HTMLDivElement> & {
    sections: ISection[];
    selectedSection?: ISection;
    onSelected?: (section: ISection | undefined) => void;
};

export default (props: ISectionSelectorProps) => {
    const updateSection = useCallback(
        (update: any) => {
            if (props.onSelected)
                props.onSelected({ ...props.selectedSection, ...update });
        },
        [props.onSelected, props.sections]
    );

    useEffect(() => {
        if (props.onSelected)
            props.onSelected({
                ...props.sections[0],
                ...props.selectedSection
            });
    }, [props.sections]);

    interface IOption {
        value?: string;
        label?: string;
    }

    const makeValue = useCallback(
        (value: string | undefined): IOption => ({
            value: value,
            label: value
        }),
        []
    );

    const makeComponents = useCallback(
        <TGroup extends GroupBase<IOption>>(
            icon: IconProp,
            classNames: ClassNamesArg = ""
        ): SelectComponentsConfig<IOption, false, TGroup> => ({
            Control: ({ children, ...rest }) => (
                <daisyui.Dropdown>
                    <components.Control
                        className={
                            "input input-bordered input-sm bg-base-100 " +
                            classNames
                        }
                        {...rest}
                    >
                        <FontAwesomeIcon className="px-2" icon={icon} />
                        {children}
                    </components.Control>
                </daisyui.Dropdown>
            ),
            Menu: ({ children, innerProps, ...rest }) => (
                <daisyui.Dropdown.Menu
                    className="z-10 fixed bg-base-100 menu-sm"
                    {...rest}
                >
                    <div {...innerProps}>{children}</div>
                </daisyui.Dropdown.Menu>
            ),
            Option: ({ children, innerProps, isSelected, ...rest }) => (
                <daisyui.Menu.Item {...rest}>
                    <div className={isSelected ? "active" : ""} {...innerProps}>
                        {children}
                    </div>
                </daisyui.Menu.Item>
            )
        }),
        []
    );

    const styles: StylesConfig<IOption> = {
        control: (base) => ({
            ...base,
            minHeight: 32
        }),
        dropdownIndicator: (base) => ({
            ...base,
            paddingTop: 0,
            paddingBottom: 0
        }),
        clearIndicator: (base) => ({
            ...base,
            paddingTop: 0,
            paddingBottom: 0
        })
    };

    return (
        <span className="flex justify-stretch items-center w-full space-x-1">
            <Select<IOption>
                unstyled={true}
                components={makeComponents(faCodeBranch)}
                styles={styles}
                value={makeValue(props.selectedSection?.branch)}
                options={branches(props.sections).map(makeValue)}
                onChange={(value) => updateSection({ branch: value?.value })}
            />
            <Select<IOption>
                unstyled={true}
                components={makeComponents(faCubes)}
                styles={styles}
                value={makeValue(props.selectedSection?.repository)}
                options={reposForBranch(
                    props.sections,
                    props.selectedSection?.branch
                ).map(makeValue)}
                onChange={(value) =>
                    updateSection({ repository: value?.value })
                }
            />
            <Select<IOption>
                unstyled={true}
                components={makeComponents(faMicrochip)}
                styles={styles}
                value={makeValue(props.selectedSection?.architecture)}
                options={architecturesForBranchAndRepo(
                    props.sections,
                    props.selectedSection?.branch,
                    props.selectedSection?.repository
                ).map(makeValue)}
                onChange={(value) =>
                    updateSection({ architecture: value?.value })
                }
            />
        </span>
    );
};
